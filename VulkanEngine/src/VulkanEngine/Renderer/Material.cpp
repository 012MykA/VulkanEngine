#include "Material.hpp"
#include "VulkanEngine/Vulkan/VulkanAllocator.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    Material::~Material()
    {
        if (m_Device)
            m_Device->WaitIdle();
    }
    
    void Material::Build(
        const VulkanAllocator &allocator,
        const VulkanLogicalDevice &device,
        const VulkanDescriptorPool &pool,
        const VulkanDescriptorSetLayout &layout)
    {
        m_Device = &device;

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
    }

    void Material::UpdateGPU()
    {
        if (m_UBO)
        {
            m_UBO->Upload(&m_Data, sizeof(MaterialData));
            VE_CORE_TRACE("Material '{}' uploaded", m_Name);
        }
    }

} // namespace ve
