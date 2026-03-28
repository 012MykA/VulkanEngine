#include "Material.hpp"
#include "VulkanEngine/Vulkan/VulkanAllocator.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    VulkanDescriptorSetLayout Material::CreateLayout(const VulkanLogicalDevice &logicalDevice)
    {
        constexpr VkShaderStageFlags kFragStage = VK_SHADER_STAGE_FRAGMENT_BIT;

        return VulkanDescriptorSetLayout::Builder(logicalDevice)
            .AddBinding(MaterialBindings::UBO, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, kFragStage)
            .AddBinding(MaterialBindings::Albedo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kFragStage)
            .AddBinding(MaterialBindings::Normal, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kFragStage)
            .AddBinding(MaterialBindings::MetallicRoughness, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kFragStage)
            .AddBinding(MaterialBindings::Emissive, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kFragStage)
            .AddBinding(MaterialBindings::Occlusion, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kFragStage)
            .Build();
    }

    void Material::Build(const VulkanAllocator &allocator,
                         const VulkanLogicalDevice &logicalDevice,
                         const VulkanDescriptorPool &pool,
                         const VulkanDescriptorSetLayout &layout,
                         const Texture &defaultWhite,
                         const Texture &defaultNormal)
    {
        m_UBOBuffer = std::make_unique<VulkanBuffer>(allocator, MakeUniformBufferDesc(sizeof(MaterialUBO)));
        m_DescriptorSet = pool.Allocate(layout.GetVkHandle());

        auto albedoInfo = (m_AlbedoMap ? *m_AlbedoMap : defaultWhite).GetDescriptorInfo();
        auto normalInfo = (m_NormalMap ? *m_NormalMap : defaultNormal).GetDescriptorInfo();
        auto mrInfo = (m_MetallicRoughnessMap ? *m_MetallicRoughnessMap : defaultWhite).GetDescriptorInfo();
        auto emissiveInfo = (m_EmissiveMap ? *m_EmissiveMap : defaultWhite).GetDescriptorInfo();
        auto occlusionInfo = (m_OcclusionMap ? *m_OcclusionMap : defaultWhite).GetDescriptorInfo();

        VkDescriptorBufferInfo uboInfo{
            .buffer = m_UBOBuffer->GetVkHandle(),
            .offset = 0,
            .range = sizeof(MaterialUBO),
        };

        VulkanDescriptorWriter(logicalDevice.GetVkHandle())
            .WriteBuffer(MaterialBindings::UBO, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_DescriptorSet, uboInfo)
            .WriteImage(MaterialBindings::Albedo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, albedoInfo)
            .WriteImage(MaterialBindings::Normal, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, normalInfo)
            .WriteImage(MaterialBindings::MetallicRoughness, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, mrInfo)
            .WriteImage(MaterialBindings::Emissive, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, emissiveInfo)
            .WriteImage(MaterialBindings::Occlusion, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_DescriptorSet, occlusionInfo)
            .Flush();

        VE_CORE_TRACE("Material '{}' built (mode={})", m_Name,
                      m_AlphaMode == AlphaMode::Opaque ? "Opaque" : m_AlphaMode == AlphaMode::Mask ? "Mask"
                                                                                                   : "Blend");
    }

    void Material::UpdateUBO()
    {
        if (m_UBOBuffer)
            m_UBOBuffer->Upload(m_UBO);
    }

    // Setters
    void Material::SetAlbedoMap(std::shared_ptr<Texture> tex)
    {
        m_AlbedoMap = std::move(tex);
        m_UBO.hasAlbedoMap = 1;
    }

    void Material::SetNormalMap(std::shared_ptr<Texture> tex)
    {
        m_NormalMap = std::move(tex);
        m_UBO.hasNormalMap = 1;
    }

    void Material::SetMetallicRoughnessMap(std::shared_ptr<Texture> tex)
    {
        m_MetallicRoughnessMap = std::move(tex);
        m_UBO.hasMetallicRoughnessMap = 1;
    }

    void Material::SetEmissiveMap(std::shared_ptr<Texture> tex)
    {
        m_EmissiveMap = std::move(tex);
        m_UBO.hasEmissiveMap = 1;
    }

    void Material::SetOcclusionMap(std::shared_ptr<Texture> tex)
    {
        m_OcclusionMap = std::move(tex);
        m_UBO.hasOcclusionMap = 1;
    }

    void Material::SetAlphaMode(AlphaMode mode)
    {
        m_AlphaMode = mode;
        m_UBO.alphaMode = static_cast<int>(mode);
    }

} // namespace ve
