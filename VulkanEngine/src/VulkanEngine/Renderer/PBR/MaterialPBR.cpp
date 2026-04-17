#include "MaterialPBR.hpp"
#include "Backends/Vulkan/VulkanAllocator.hpp"
#include "Backends/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Core/Timer.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <stb_image.h>

#include <array>
#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <future>

namespace ve
{
    enum class MapType
    {
        BaseColor,
        Emissive,
        Roughness,
        Metallic,
        Normal,
        Occlusion,
        Unknown
    };

    // Holds CPU pixel data decoded by a worker thread
    // GPU upload happens on the main thread
    struct DecodedImage
    {
        MapType type = MapType::Unknown;
        std::string filepath;
        TextureFormat format;
        bool generateMips = true;

        void *pixels = nullptr; // allocated by stb_image and freed after upload
        int width = 0;
        int height = 0;
    };

    std::shared_ptr<MaterialPBR> MaterialPBR::Load(const std::string &path,
                                                   const VulkanAllocator &allocator,
                                                   const VulkanLogicalDevice &logicalDevice,
                                                   const VulkanImmediateSubmit &upload,
                                                   bool generateMips)
    {
        Timer timer;

        if (!std::filesystem::exists(path))
            throw std::runtime_error("failed to load PBR material: path '{}' " + path + " does not exists");

        if (!std::filesystem::is_directory(path))
            throw std::runtime_error("failed to load PBR material: '{}' " + path + " should be a folder");

        auto mat = std::make_shared<MaterialPBR>();

        std::string matName = std::filesystem::path(path).filename().string();
        mat->SetName(matName);

        struct DecodeTask
        {
            MapType type;
            std::future<DecodedImage> future;
        };

        std::vector<DecodeTask> tasks;

        for (const auto &entry : std::filesystem::directory_iterator(path))
        {
            if (!entry.is_regular_file())
                continue;

            std::string filepath = entry.path().string();
            std::string filename = entry.path().filename().string();
            std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

            MapType type = MapType::Unknown;
            TextureFormat format = TextureFormat::RGBA8_UNORM;

            if (filename.contains("albedo") || filename.contains("basecolor"))
            {
                type = MapType::BaseColor;
                format = TextureFormat::RGBA8_SRGB;
            }
            else if (filename.contains("emissive"))
            {
                type = MapType::Emissive;
                format = TextureFormat::RGBA8_SRGB;
            }
            else if (filename.contains("normal"))
            {
                type = MapType::Normal;
                format = TextureFormat::RGBA8_UNORM;
            }
            else if (filename.contains("metallic"))
            {
                type = MapType::Metallic;
                format = TextureFormat::RGBA8_UNORM;
            }
            else if (filename.contains("roughness"))
            {
                type = MapType::Roughness;
                format = TextureFormat::RGBA8_UNORM;
            }
            else if (filename.contains("ao") || filename.contains("occlusion"))
            {
                type = MapType::Occlusion;
                format = TextureFormat::RGBA8_UNORM;
            }
            else
            {
                type = MapType::Unknown;
            }

            if (type == MapType::Unknown)
                continue;

            // clang-format off
            tasks.push_back(DecodeTask{
                .type = type,
                .future = std::async(std::launch::async,
                    [filepath, format, generateMips]() -> DecodedImage
                    {
                        DecodedImage img;
                        img.filepath = filepath;
                        img.format = format;
                        img.generateMips = generateMips;
                        img.type = MapType::Unknown;

                        int ch;
                        if (stbi_is_hdr(filepath.c_str()))
                        {
                            img.pixels = stbi_loadf(filepath.c_str(), &img.width, &img.height, &ch, STBI_rgb_alpha);
                            img.format = TextureFormat::RGBA32_SFLOAT;
                        }
                        else
                        {
                            img.pixels = stbi_load(filepath.c_str(), &img.width, &img.height, &ch, STBI_rgb_alpha);
                        }

                        if (img.pixels)
                            img.type = format == TextureFormat::RGBA8_SRGB
                                           ? img.type
                                           : img.type;
                        return img;
                    }),
            });
            
            tasks.back().type = type;
            // clang-format on
        }

        // Collecting results
        for (auto &task : tasks)
        {

            DecodedImage img = task.future.get();

            if (!img.pixels)
            {
                VE_CORE_ERROR("MaterialPBR: failed to decode '{}'", img.filepath);
                continue;
            }

            img.type = task.type;

            auto tex = std::make_shared<Texture>();
            TextureDesc desc{.format = img.format, .generateMips = img.generateMips};
            tex->InitializeAndUpload(img.pixels,
                                     static_cast<uint32_t>(img.width),
                                     static_cast<uint32_t>(img.height),
                                     desc, allocator, logicalDevice, upload);
            stbi_image_free(img.pixels);

            if (!tex)
                continue;

            // clang-format off
            switch (img.type)
            {
            case MapType::BaseColor:    mat->SetBaseColorMap(tex);  break;
            case MapType::Emissive:     mat->SetEmissiveMap(tex);   break;
            case MapType::Metallic:     mat->SetMetallicMap(tex);   break;
            case MapType::Roughness:    mat->SetRoughnessMap(tex);  break;
            case MapType::Normal:       mat->SetNormalMap(tex);     break;
            case MapType::Occlusion:    mat->SetOcclusionMap(tex);  break;
            default: break;
            }
            // clang-format on
        }

        VE_CORE_INFO("PBR Material '{}' loaded ({} ms)", matName, timer.ElapsedMilliseconds());

        return mat;
    }

    void MaterialPBR::Build(const VulkanAllocator &allocator,
                            const VulkanLogicalDevice &logicalDevice,
                            const VulkanDescriptorPool &pool,
                            const VulkanDescriptorSetLayout &layout,
                            const Texture &defaultWhite,
                            const Texture &defaultNormalMap)
    {
        assert(m_DescriptorSet == VK_NULL_HANDLE && "Descriptor Set is already initialized. Use MaterialPBR::UpdateGPU to update data");

        m_UBO = std::make_unique<VulkanBuffer>(allocator, MakeUniformBufferDesc(sizeof(MaterialPBRData)));
        m_UBO->Upload(&m_Data, sizeof(MaterialPBRData));

        m_DescriptorSet = pool.Allocate(layout.GetVkHandle());

        VkDescriptorBufferInfo bufferInfo{
            .buffer = m_UBO->GetVkHandle(),
            .offset = 0,
            .range = sizeof(MaterialPBRData),
        };

        VkDescriptorImageInfo baseColorMapInfo = (m_BaseColorMap ? *m_BaseColorMap : defaultWhite).GetDescriptorInfo();
        VkDescriptorImageInfo emissiveMapInfo = (m_EmissiveMap ? *m_EmissiveMap : defaultWhite).GetDescriptorInfo();
        VkDescriptorImageInfo metallicMapInfo = (m_MetallicMap ? *m_MetallicMap : defaultWhite).GetDescriptorInfo();
        VkDescriptorImageInfo roughnessMapInfo = (m_RoughnessMap ? *m_RoughnessMap : defaultWhite).GetDescriptorInfo();
        VkDescriptorImageInfo normalMapInfo = (m_NormalMap ? *m_NormalMap : defaultNormalMap).GetDescriptorInfo();
        VkDescriptorImageInfo occlusionMapInfo = (m_OcclusionMap ? *m_OcclusionMap : defaultWhite).GetDescriptorInfo();

        VulkanDescriptorWriter(logicalDevice.GetVkHandle())
            .WriteBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_DescriptorSet, bufferInfo)
            .WriteImage(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, baseColorMapInfo)
            .WriteImage(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, emissiveMapInfo)
            .WriteImage(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, metallicMapInfo)
            .WriteImage(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, roughnessMapInfo)
            .WriteImage(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, normalMapInfo)
            .WriteImage(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, occlusionMapInfo)
            .Flush();
    }

    void MaterialPBR::UpdateGPU()
    {
        if (m_UBO)
            m_UBO->Upload(&m_Data, sizeof(MaterialPBRData));
    }

    VulkanDescriptorSetLayout MaterialPBR::CreateLayout(const VulkanLogicalDevice &device)
    {
        return VulkanDescriptorSetLayout::Builder(device)
            // Binding 0: Factors, Indices
            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            // Bindings 1-6: Textures
            .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build();
    }

    void MaterialPBR::SetName(const std::string &name) { m_Name = name; }

    void MaterialPBR::SetBaseColor(const glm::vec4 &color) { m_Data.baseColorFactor = color; }

    void MaterialPBR::SetEmissive(const glm::vec3 &color) { m_Data.emissiveFactor = color; }

    void MaterialPBR::SetMetallic(float v) { m_Data.metallicFactor = v; }

    void MaterialPBR::SetRoughness(float v) { m_Data.roughnessFactor = v; }

    void MaterialPBR::SetBaseColorMap(std::shared_ptr<Texture> tex)
    {
        m_BaseColorMap = std::move(tex);
        m_Data.baseColorTextureIdx = (m_BaseColorMap ? 1 : -1);
    }

    void MaterialPBR::SetEmissiveMap(std::shared_ptr<Texture> tex)
    {
        m_EmissiveMap = std::move(tex);
        m_Data.emissiveTextureIdx = (m_EmissiveMap ? 1 : -1);
    }

    void MaterialPBR::SetMetallicMap(std::shared_ptr<Texture> tex)
    {
        m_MetallicMap = std::move(tex);
        m_Data.metallicTextureIdx = (m_MetallicMap ? 1 : -1);
    }

    void MaterialPBR::SetRoughnessMap(std::shared_ptr<Texture> tex)
    {
        m_RoughnessMap = std::move(tex);
        m_Data.roughnessTextureIdx = (m_RoughnessMap ? 1 : -1);
    }

    void MaterialPBR::SetNormalMap(std::shared_ptr<Texture> tex)
    {
        m_NormalMap = std::move(tex);
        m_Data.normalTextureIdx = (m_NormalMap ? 1 : -1);
    }

    void MaterialPBR::SetOcclusionMap(std::shared_ptr<Texture> tex)
    {
        m_OcclusionMap = std::move(tex);
        m_Data.occlusionTextureIdx = (m_OcclusionMap ? 1 : -1);
    }

} // namespace ve
