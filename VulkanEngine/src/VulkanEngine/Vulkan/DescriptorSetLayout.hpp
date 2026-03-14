#pragma once

#include "DescriptorBinding.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace ve
{
    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout(VkDevice device, const std::vector<DescriptorBinding> &descriptorBindings);
        ~DescriptorSetLayout();

        VkDescriptorSetLayout GetLayout() const { return m_Layout; }

    private:
        VkDevice m_Device;
        VkDescriptorSetLayout m_Layout;
    };

} // namespace ve
