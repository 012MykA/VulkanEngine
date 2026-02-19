#pragma once

#include "DescriptorBinding.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace VE
{
    class Device;

    class DescriptorPool
    {
    public:
        DescriptorPool(const Device &device, const std::vector<DescriptorBinding> &descriptorBindings, const size_t maxSets);
        ~DescriptorPool();

        VkDescriptorPool Handle() const { return m_DescriptorPool; }

        const Device &GetDevice() const { return m_Device; }

    private:
        const Device &m_Device;
        VkDescriptorPool m_DescriptorPool;
    };

}
