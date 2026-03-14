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

        VkDescriptorPool GetPool() const { return m_Pool; }

    private:
        VkDevice m_Device;
        VkDescriptorPool m_Pool = VK_NULL_HANDLE;
    };

} // namespace ve
