#pragma once

#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <map>
#include <cstdint>

namespace ve
{
    using DescriptorBindingTypesMap = std::map<uint32_t, VkDescriptorType>;

    class DescriptorSets
    {
    public:
        DescriptorSets(const DescriptorPool &descriptorPool, const DescriptorSetLayout &layout,
                       DescriptorBindingTypesMap bindingTypes, const size_t size);
        ~DescriptorSets();

        void UpdateDescriptors(const std::vector<VkWriteDescriptorSet> &descriptorWrites);

        VkWriteDescriptorSet Bind(const size_t index, const uint32_t binding,
                                  const VkDescriptorBufferInfo &bufferInfo, const uint32_t count = 1) const;

    public:
        // Getters
        VkDescriptorSet operator[](uint32_t index) { return m_DescriptorSets[index]; }

    private:
        VkDescriptorType GetBindingType(const uint32_t binding) const;

    private:
        const DescriptorPool &m_DescriptorPool;

        std::vector<VkDescriptorSet> m_DescriptorSets;
        const DescriptorBindingTypesMap m_BindingTypes;
    };

} // namespace ve
