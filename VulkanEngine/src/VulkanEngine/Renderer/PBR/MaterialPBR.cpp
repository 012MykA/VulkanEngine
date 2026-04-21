#include "MaterialPBR.hpp"
#include "Backends/Vulkan/VulkanAllocator.hpp"
#include "Backends/Vulkan/VulkanDescriptor.hpp"
#include "Backends/Vulkan/VulkanLogicalDevice.hpp"

#include <cassert>

namespace ve
{
    void MaterialPBR::Upload(const VulkanAllocator &allocator,
                             const VulkanDescriptorPool &pool,
                             const VulkanDescriptorSetLayout &layout,
                             const VulkanLogicalDevice &logicalDevice,
                             const Texture &defaultWhite,
                             const Texture &defaultNormal)
    {
        assert(m_DescriptorSet == VK_NULL_HANDLE && "Descriptor Set is already initialized");

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
        VkDescriptorImageInfo metallicRoughnessMapInfo = (m_MetallicRoughnessMap ? *m_MetallicRoughnessMap : defaultWhite).GetDescriptorInfo();
        VkDescriptorImageInfo normalMapInfo = (m_NormalMap ? *m_NormalMap : defaultNormal).GetDescriptorInfo();
        VkDescriptorImageInfo occlusionMapInfo = (m_OcclusionMap ? *m_OcclusionMap : defaultWhite).GetDescriptorInfo();

        // clang-format off
        VulkanDescriptorWriter(logicalDevice.GetVkHandle())
            .WriteBuffer(Binding::BufferInfo,       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          m_DescriptorSet, bufferInfo)
            .WriteImage(Binding::BaseColor,         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  m_DescriptorSet, baseColorMapInfo)
            .WriteImage(Binding::Emissive,          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  m_DescriptorSet, emissiveMapInfo)
            .WriteImage(Binding::MetallicRoughness, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  m_DescriptorSet, metallicRoughnessMapInfo)
            .WriteImage(Binding::Normal,            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  m_DescriptorSet, normalMapInfo)
            .WriteImage(Binding::Occlusion,         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  m_DescriptorSet, occlusionMapInfo)
            .Flush();
        // clang-format on
    }

    void MaterialPBR::UpdateGPU()
    {
        assert(m_UBO && m_DescriptorSet != VK_NULL_HANDLE && "MaterialPBR must be uploaded. Use MaterialPBR::Upload");

        m_UBO->Upload(&m_Data, sizeof(MaterialPBRData));
    }

    void MaterialPBR::Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t setIndex) const
    {
        assert(m_DescriptorSet != VK_NULL_HANDLE && "Material must be uploaded. Use MaterialPBR::Upload");

        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            setIndex,
            1, &m_DescriptorSet,
            0, nullptr);
    }

    VulkanDescriptorSetLayout MaterialPBR::CreateLayout(const VulkanLogicalDevice &device)
    {
        // clang-format off
        return VulkanDescriptorSetLayout::Builder(device)
            .AddBinding(Binding::BufferInfo,        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(Binding::BaseColor,         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(Binding::Emissive,          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(Binding::MetallicRoughness, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(Binding::Normal,            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(Binding::Occlusion,         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build();
        // clang-format on
    }

} // namespace ve
