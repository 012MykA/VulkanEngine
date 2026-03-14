#include "DescriptorSets.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanValidation.hpp"

#include <stdexcept>

namespace ve
{
    DescriptorSets::DescriptorSets(
        const DescriptorPool &descriptorPool, const DescriptorSetLayout &layout,
        DescriptorBindingTypes bindingTypes, const size_t size) : m_DescriptorPool(descriptorPool)
    {
        std::vector<VkDescriptorSetLayout> layouts(size, layout.GetLayout());

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool.GetPool();
        allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
        allocInfo.pSetLayouts = layouts.data();

        m_DescriptorSets.resize(size);

        VkResult result = vkAllocateDescriptorSets(m_DescriptorPool.GetDevice(), &allocInfo, m_DescriptorSets.data());
        CHECK_VK_RESULT(result);
    }

    DescriptorSets::~DescriptorSets()
    {
        // vkFreeDescriptorSets(
        //     m_DescriptorPool.GetDevice(),
        //     m_DescriptorPool.GetPool(),
        //     static_cast<uint32_t>(m_DescriptorSets.size()),
        //     m_DescriptorSets.data());

        // m_DescriptorSets.clear();
    }

    VkWriteDescriptorSet DescriptorSets::Bind(const size_t index, const uint32_t binding,
                                              const VkDescriptorBufferInfo &bufferInfo, const uint32_t count) const
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSets[index];
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = GetBindingType(binding);
        descriptorWrite.descriptorCount = count;
        descriptorWrite.pBufferInfo = &bufferInfo;

        return descriptorWrite;
    }

    VkDescriptorType DescriptorSets::GetBindingType(const uint32_t binding) const
    {
        const auto it = m_BindingTypes.find(binding);
        if (it == m_BindingTypes.end())
            throw std::invalid_argument("binding not found");

        return it->second;
    }

} // namespace ve
