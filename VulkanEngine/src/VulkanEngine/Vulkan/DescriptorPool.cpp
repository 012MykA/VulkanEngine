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
            VkDescriptorPoolSize poolSize{};
            poolSize.type = b.type;
            poolSize.descriptorCount = static_cast<uint32_t>(b.descriptorCount * maxSets);
            poolSizes.push_back(poolSize);
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(maxSets);

        VkResult result = vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_Pool);
        CHECK_VK_RESULT(result);
        VE_CORE_TRACE("VkDescriptorPool created");
    }

    DescriptorPool::~DescriptorPool()
    {
        vkDestroyDescriptorPool(m_Device, m_Pool, nullptr);
        VE_CORE_TRACE("VkDescriptorPool destroy");
    }

} // namespace ve
