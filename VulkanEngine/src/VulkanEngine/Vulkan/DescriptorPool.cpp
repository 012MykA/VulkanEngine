#include "DescriptorPool.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    DescriptorPool::DescriptorPool(VkDevice device, const std::vector<DescriptorBinding> &descriptorBindings, const size_t maxSets)
        : m_Device(device)
    {
        std::vector<VkDescriptorPoolSize> poolSizes;

        for (const auto &b : descriptorBindings)
        {
            VkDescriptorPoolSize poolSize{
                .type = b.Type,
                .descriptorCount = static_cast<uint32_t>(b.DescriptorCount * maxSets),
            };
            poolSizes.push_back(poolSize);
        }

        VkDescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(maxSets),
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data(),
        };

        VkResult result = vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool);
        CHECK_VK_RESULT(result);
        VE_CORE_TRACE("VkDescriptorPool created");
    }

    DescriptorPool::~DescriptorPool()
    {
        vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
        VE_CORE_TRACE("VkDescriptorPool destroy");
    }

} // namespace ve
