#include "DescriptorSets.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "Validation.hpp"

#include <stdexcept>

namespace VE
{
    DescriptorSets::DescriptorSets(const DescriptorPool &descriptorPool, const DescriptorSetLayout &layout,
                                   std::map<uint32_t, VkDescriptorType> bindingTypes, const size_t size)
        : m_DescriptorPool(descriptorPool), m_BindingTypes(std::move(bindingTypes))
    {
        std::vector<VkDescriptorSetLayout> layouts(size, layout.Handle());

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool.Handle();
        allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
        allocInfo.pSetLayouts = layouts.data();

        m_DescriptorSets.resize(size);
        CheckVk(vkAllocateDescriptorSets(descriptorPool.GetDevice().Handle(), &allocInfo, m_DescriptorSets.data()),
                "allocate descriptor sets!");
    }

    DescriptorSets::~DescriptorSets() = default;

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
        descriptorWrite.pImageInfo = nullptr;       // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        return descriptorWrite;
    }

    VkWriteDescriptorSet DescriptorSets::Bind(const size_t index, const uint32_t binding, const VkDescriptorImageInfo &imageInfo, const uint32_t count) const
    {
        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSets[index];
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = GetBindingType(binding);
        descriptorWrite.descriptorCount = count;
        descriptorWrite.pImageInfo = &imageInfo;

        return descriptorWrite;
    }

    void DescriptorSets::UpdateDescriptors(const std::vector<VkWriteDescriptorSet> &descriptorWrites)
    {
        vkUpdateDescriptorSets(m_DescriptorPool.GetDevice().Handle(),
                               static_cast<uint32_t>(descriptorWrites.size()),
                               descriptorWrites.data(), 0, nullptr);
    }

    VkDescriptorType DescriptorSets::GetBindingType(const uint32_t binding) const
    {
        const auto it = m_BindingTypes.find(binding);
        if (it == m_BindingTypes.end())
            throw std::runtime_error("failed to find binding");

        return it->second;
    }

}
