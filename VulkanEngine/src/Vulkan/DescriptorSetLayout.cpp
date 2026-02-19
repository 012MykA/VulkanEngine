#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "Validation.hpp"

namespace VE
{
    DescriptorSetLayout::DescriptorSetLayout(const Device &device, const std::vector<DescriptorBinding> &descriptorBindings)
        : m_Device(device)
    {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(descriptorBindings.size());

        for (const auto &binding : descriptorBindings)
        {
            VkDescriptorSetLayoutBinding b{};
            b.binding = binding.Binding;
            b.descriptorType = binding.Type;
            b.descriptorCount = binding.DescriptorCount;
            b.stageFlags = binding.Stage;
            b.pImmutableSamplers = nullptr; // Optional

            layoutBindings.push_back(b);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        CheckVk(vkCreateDescriptorSetLayout(m_Device.Handle(), &layoutInfo, nullptr, &m_Layout),
                "create descriptor set layout!");
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(m_Device.Handle(), m_Layout, nullptr);
    }

}
