#pragma once

#include "VulkanEngine/Core/Base.hpp"

#include "DescriptorBinding.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "DescriptorSets.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <cassert>

namespace ve
{
    class DescriptorSetManager
    {
    public:
        DescriptorSetManager(VkDevice device, const std::vector<DescriptorBinding> &descriptorBindings, const size_t maxSets);
        ~DescriptorSetManager();

        const DescriptorSetLayout &GetDescriptorSetLayout() const
        {
            assert(m_DescriptorSetLayout != nullptr);
            return *m_DescriptorSetLayout;
        }
        DescriptorSets &GetSets()
        {
            assert(m_DescriptorSets != nullptr);
            return *m_DescriptorSets;
        }

    private:
        Scope<DescriptorPool> m_DescriptorPool;
        Scope<DescriptorSetLayout> m_DescriptorSetLayout;
        Scope<DescriptorSets> m_DescriptorSets;
    };

} // namespace ve
