#pragma once

#include "DescriptorBinding.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace ve
{
    class DescriptorPool
    {
    public:
        DescriptorPool(VkDevice device, const std::vector<DescriptorBinding> &descriptorBindings, const size_t maxSets);
        ~DescriptorPool();

        VkDescriptorPool GetPool() const { return m_DescriptorPool; }
        VkDevice GetDevice() const { return m_Device; }

    private:
        VkDevice m_Device;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    };

} // namespace ve
