#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <cstdint>

namespace VE
{
    class Device;

    struct DescriptorBinding
    {
        uint32_t Binding;
        uint32_t DescriptorCount;
        VkDescriptorType Type;
        VkShaderStageFlags Stage;
    };

    class DescriptorSetLayout final
    {
    public:
        DescriptorSetLayout(const Device &device, const std::vector<DescriptorBinding> &descriptorBindings);
        ~DescriptorSetLayout();

        VkDescriptorSetLayout Handle() const { return m_Layout; }

    private:
        const Device &m_Device;
        VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
    };

}
