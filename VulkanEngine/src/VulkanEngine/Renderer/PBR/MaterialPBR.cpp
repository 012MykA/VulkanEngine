#include "MaterialPBR.hpp"
#include "VulkanEngine/Vulkan/VulkanAllocator.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <array>

namespace ve
{
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
        VkDescriptorImageInfo metallicRoughnessMapInfo = (m_MetallicRoughnessMap ? *m_MetallicRoughnessMap : defaultWhite).GetDescriptorInfo();
        VkDescriptorImageInfo normalMapInfo = (m_NormalMap ? *m_NormalMap : defaultNormalMap).GetDescriptorInfo();
        VkDescriptorImageInfo occlusionMapInfo = (m_OcclusionMap ? *m_OcclusionMap : defaultWhite).GetDescriptorInfo();
        VkDescriptorImageInfo emissiveMapInfo = (m_EmissiveMap ? *m_EmissiveMap : defaultWhite).GetDescriptorInfo();

        VulkanDescriptorWriter(logicalDevice.GetVkHandle())
            .WriteBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_DescriptorSet, bufferInfo)
            .WriteImage(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, baseColorMapInfo)
            .WriteImage(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, metallicRoughnessMapInfo)
            .WriteImage(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, normalMapInfo)
            .WriteImage(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, occlusionMapInfo)
            .WriteImage(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, emissiveMapInfo)
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
            // Bindings 1-5: Textures (BaseColor, MetRoug, Normal, Occlusion, Emissive)
            .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
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

    void MaterialPBR::SetMetallicRoughnessMap(std::shared_ptr<Texture> tex)
    {
        m_MetallicRoughnessMap = std::move(tex);
        m_Data.metallicRoughnessTextureIdx = (m_MetallicRoughnessMap ? 1 : -1);
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

    void MaterialPBR::SetEmissiveMap(std::shared_ptr<Texture> tex)
    {
        m_EmissiveMap = std::move(tex);
        m_Data.emissiveTextureIdx = (m_EmissiveMap ? 1 : -1);
    }

} // namespace ve
