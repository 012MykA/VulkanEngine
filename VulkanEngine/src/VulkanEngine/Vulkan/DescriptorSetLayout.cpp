#include "DescriptorSetLayout.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanValidation.hpp"

#include <cstdint>

namespace ve
{
    DescriptorSetLayout::DescriptorSetLayout(VkDevice device, const std::vector<DescriptorBinding> &descriptorBindings)
        : m_Device(device)
    {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

        for (const auto &b : descriptorBindings)
        {
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = b.binding;
            binding.descriptorCount = b.descriptorCount;
            binding.stageFlags = binding.stageFlags;

            layoutBindings.push_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        VkResult result = vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_Layout);
        CHECK_VK_RESULT(result);

        VE_CORE_TRACE("VkDesciprtorSetLayout created");
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(m_Device, m_Layout, nullptr);
        VE_CORE_TRACE("VkDesciprtorSetLayout destroyed");
    }

} // namespace ve
