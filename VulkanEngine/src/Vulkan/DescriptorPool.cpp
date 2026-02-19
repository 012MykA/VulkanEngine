#include "DescriptorPool.hpp"
#include "Device.hpp"
#include "Validation.hpp"

namespace VE
{
    DescriptorPool::DescriptorPool(const Device &device, const std::vector<DescriptorBinding> &descriptorBindings, const size_t maxSets)
        : m_Device(device)
    {
        std::vector<VkDescriptorPoolSize> poolSizes;
        poolSizes.reserve(descriptorBindings.size());

        for (const auto &binding : descriptorBindings)
        {
            VkDescriptorPoolSize s{};
            s.type = binding.Type;
            s.descriptorCount = static_cast<uint32_t>(binding.DescriptorCount * maxSets);

            poolSizes.push_back(s);
        }

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(maxSets);

        CheckVk(vkCreateDescriptorPool(device.Handle(), &poolInfo, nullptr, &m_DescriptorPool),
                "create descriptor pool!");
    }

    DescriptorPool::~DescriptorPool()
    {
        vkDestroyDescriptorPool(m_Device.Handle(), m_DescriptorPool, nullptr);
    }

}
