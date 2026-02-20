#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <cstdint>
#include <map>

namespace VE
{
    class DescriptorPool;
    class DescriptorSetLayout;

    class DescriptorSets
    {
    public:
        DescriptorSets(const DescriptorPool &descriptorPool, const DescriptorSetLayout &layout,
                       std::map<uint32_t, VkDescriptorType> bindingTypes, const size_t size);
        ~DescriptorSets();

        VkDescriptorSet operator[](size_t index) const { return m_DescriptorSets[index]; }

        VkWriteDescriptorSet Bind(const size_t index, const uint32_t binding,
                                  const VkDescriptorBufferInfo &bufferInfo, const uint32_t count) const;

        VkWriteDescriptorSet Bind(const size_t index, const uint32_t binding,
                                                  const VkDescriptorImageInfo &imageInfo, const uint32_t count) const;

        void UpdateDescriptors(const std::vector<VkWriteDescriptorSet> &descriptorWrites);

    private:
        VkDescriptorType GetBindingType(const uint32_t binding) const;

    private:
        const DescriptorPool &m_DescriptorPool;

        const std::map<uint32_t, VkDescriptorType> m_BindingTypes;
        std::vector<VkDescriptorSet> m_DescriptorSets;
    };

}
