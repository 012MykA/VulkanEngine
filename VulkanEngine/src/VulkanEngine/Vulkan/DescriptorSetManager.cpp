#include "DescriptorSetManager.hpp"

#include <stdexcept>

namespace ve
{
    DescriptorSetManager::DescriptorSetManager(VkDevice device, const std::vector<DescriptorBinding> &descriptorBindings, const size_t maxSets)
    {
        DescriptorBindingTypesMap bindingTypes;

        for (const auto &b : descriptorBindings)
        {
            if (!bindingTypes.insert(std::make_pair(b.binding, b.type)).second)
            {
                throw std::invalid_argument("binding collision");
            }
        }

        m_DescriptorPool = CreateScope<DescriptorPool>(device, descriptorBindings, maxSets);
        m_DescriptorSetLayout = CreateScope<DescriptorSetLayout>(device, descriptorBindings);
        m_DescriptorSets = CreateScope<DescriptorSets>(*m_DescriptorPool, *m_DescriptorSetLayout, bindingTypes, maxSets);
    }

    DescriptorSetManager::~DescriptorSetManager()
    {
        m_DescriptorSets.reset();
        m_DescriptorSetLayout.reset();
        m_DescriptorPool.reset();
    }

} // namespace ve
