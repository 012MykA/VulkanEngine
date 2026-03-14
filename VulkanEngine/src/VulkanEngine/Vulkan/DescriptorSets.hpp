#pragma once

#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <map>
#include <cstdint>

namespace ve
{
    using DescriptorBindingTypes = std::map<uint32_t, VkDescriptorType>;

    class DescriptorSets
    {
    public:
        DescriptorSets(const DescriptorPool &descriptorPool, const DescriptorSetLayout &layout,
                       DescriptorBindingTypes bindingTypes, const size_t size);
        ~DescriptorSets();

        VkWriteDescriptorSet Bind(const size_t index, const uint32_t binding,
                                  const VkDescriptorBufferInfo &bufferInfo, const uint32_t count) const;

    private:
        VkDescriptorType GetBindingType(const uint32_t binding) const;

    private:
        const DescriptorPool &m_DescriptorPool;

        std::vector<VkDescriptorSet> m_DescriptorSets;
        const DescriptorBindingTypes m_BindingTypes;
    };

} // namespace ve
