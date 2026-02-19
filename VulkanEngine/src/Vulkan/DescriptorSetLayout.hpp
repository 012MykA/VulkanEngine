#pragma once

#include "DescriptorBinding.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <cstdint>

namespace VE
{
    class Device;

    class DescriptorSetLayout final
    {
    public:
        DescriptorSetLayout(const Device &device, const std::vector<DescriptorBinding> &descriptorBindings);
        ~DescriptorSetLayout();

        VkDescriptorSetLayout Handle() const { return m_Layout; }

    private:
        const Device &m_Device;
        VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
    };

}
