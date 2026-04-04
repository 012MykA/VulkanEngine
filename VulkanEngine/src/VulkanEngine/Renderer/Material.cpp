#include "Material.hpp"
#include "VulkanEngine/Vulkan/VulkanAllocator.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{    
    void Material::Build(
        const VulkanAllocator &allocator,
        const VulkanLogicalDevice &device,
        const VulkanDescriptorPool &pool,
        const VulkanDescriptorSetLayout &layout)
    {
        m_UBO = std::make_unique<VulkanBuffer>(allocator, MakeUniformBufferDesc(sizeof(MaterialData)));

        m_UBO->Upload(&m_Data, sizeof(MaterialData));

        m_DescriptorSet = pool.Allocate(layout.GetVkHandle());

        VkDescriptorBufferInfo bufferInfo{
            .buffer = m_UBO->GetVkHandle(),
            .offset = 0,
            .range = sizeof(MaterialData),
        };

        VulkanDescriptorWriter(device.GetVkHandle())
            .WriteBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_DescriptorSet, bufferInfo)
            .Flush();

        VE_CORE_TRACE("Material '{}' built", m_Name);
    }

    void Material::UpdateGPU()
    {
        if (m_UBO)
            m_UBO->Upload(&m_Data, sizeof(MaterialData));
    }

} // namespace ve
