#include "VulkanDescriptor.hpp"
#include "VulkanLogicalDevice.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    // --- Builder ---
    VulkanDescriptorSetLayout::Builder::Builder(const VulkanLogicalDevice &logicalDevice)
        : m_Device(logicalDevice.GetVkHandle())
    {
    }

    VulkanDescriptorSetLayout::Builder &VulkanDescriptorSetLayout::Builder::AddBinding(
        uint32_t binding,
        VkDescriptorType type,
        VkShaderStageFlags stages,
        uint32_t count)
    {
        m_Bindings.push_back(VkDescriptorSetLayoutBinding{
            .binding = binding,
            .descriptorType = type,
            .descriptorCount = count,
            .stageFlags = stages,
            .pImmutableSamplers = nullptr,
        });
        return *this;
    }

    VulkanDescriptorSetLayout::Builder &VulkanDescriptorSetLayout::Builder::AddImmutableSamplerBinding(
        uint32_t binding,
        VkShaderStageFlags stages,
        std::vector<VkSampler> samplers)
    {
        m_ImmutableSamplers.push_back(std::move(samplers));
        const auto &stored = m_ImmutableSamplers.back();

        m_Bindings.push_back(VkDescriptorSetLayoutBinding{
            .binding = binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = static_cast<uint32_t>(stored.size()),
            .stageFlags = stages,
            .pImmutableSamplers = stored.data(),
        });
        return *this;
    }

    VulkanDescriptorSetLayout VulkanDescriptorSetLayout::Builder::Build() const
    {
        VkDescriptorSetLayoutCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(m_Bindings.size()),
            .pBindings = m_Bindings.data(),
        };

        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VkResult result = vkCreateDescriptorSetLayout(m_Device, &createInfo, nullptr, &layout);
        CHECK_VK_RESULT(result);

        return VulkanDescriptorSetLayout(m_Device, layout);
    }

    // --- VulkanDescriptorSetLayout ---

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout)
        : m_Device(device), m_Layout(layout)
    {
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
    {
        if (m_Layout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(m_Device, m_Layout, nullptr);
    }

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDescriptorSetLayout &&other) noexcept
        : m_Device(other.m_Device), m_Layout(other.m_Layout)
    {
        other.m_Layout = VK_NULL_HANDLE;
    }

    VulkanDescriptorSetLayout &VulkanDescriptorSetLayout::operator=(VulkanDescriptorSetLayout &&other) noexcept
    {
        if (this != &other)
        {
            if (m_Layout != VK_NULL_HANDLE)
                vkDestroyDescriptorSetLayout(m_Device, m_Layout, nullptr);
            m_Device = other.m_Device;
            m_Layout = other.m_Layout;
            other.m_Layout = VK_NULL_HANDLE;
        }
        return *this;
    }

} // namespace ve
