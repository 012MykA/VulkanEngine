#include "MaterialPBR.hpp"
#include "VulkanEngine/Vulkan/VulkanAllocator.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <array>
#include <filesystem>
#include <algorithm>
#include <future>

#include "VulkanEngine/Core/Timer.hpp"

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

    struct TextureTask
    {
        std::string type;
        std::future<std::shared_ptr<Texture>> futureTexture;
    };

    std::shared_ptr<MaterialPBR> MaterialPBR::Load(const std::string &path,
                                                   const VulkanAllocator &allocator,
                                                   const VulkanLogicalDevice &logicalDevice,
                                                   const VulkanImmediateSubmit &upload,
                                                   bool generateMips)
    {
        if (!std::filesystem::exists(path))
        {
            VE_CORE_ERROR("failed to load PBR material: path '{}' does not exists", path);
            return nullptr;
        }

        if (!std::filesystem::is_directory(path))
        {
            VE_CORE_ERROR("failed to load PBR material: '{}' should be a folder", path);
            return nullptr;
        }

        auto mat = std::make_shared<MaterialPBR>();

        std::string matName = std::filesystem::path(path).filename().string();
        mat->SetName(matName);

        Timer timer;

        // --- Setting loading tasks ---
        std::vector<TextureTask> tasks;

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

            if (type == MapType::Unknown)
                continue;

            // clang-format off
            tasks.push_back(TextureTask{
                .type = filename,
                .futureTexture = std::async(std::launch::async,
                    [=, &allocator, &logicalDevice, &upload]()
                    {
                        return Texture::LoadFromFile(
                            filepath,
                            TextureDesc{
                                .format = format,
                                .generateMips = generateMips,
                            },
                            allocator, logicalDevice, upload);
                    }
                ),
            });
            // clang-format on
        }

        // Collection results
        for (auto &task : tasks)
        {
            auto tex = task.futureTexture.get();
            if (!tex)
                continue;

            // clang-format off
            std::string name = task.type;
            if (name.contains("albedo") || name.contains("basecolor"))  mat->SetBaseColorMap(tex);
            else if (name.contains("emissive"))                         mat->SetEmissiveMap(tex);
            else if (name.contains("normal"))                           mat->SetNormalMap(tex);
            else if (name.contains("metallic"))                         mat->SetMetallicMap(tex);
            else if (name.contains("roughness"))                        mat->SetRoughnessMap(tex);
            else if (name.contains("ao") || name.contains("occlusion")) mat->SetOcclusionMap(tex);
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

        VE_CORE_TRACE("PBR Material '{}' built", m_Name);
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
