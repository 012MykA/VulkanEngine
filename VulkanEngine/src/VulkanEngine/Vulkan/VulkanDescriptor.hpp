#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <span>
#include <cstdint>
#include <initializer_list>

namespace ve
{
    class VulkanLogicalDevice;

    class VulkanDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            explicit Builder(const VulkanLogicalDevice &logicalDevice);

            Builder &AddBinding(uint32_t binding,
                                VkDescriptorType type,
                                VkShaderStageFlags stages,
                                uint32_t count = 1);

            Builder &AddImmutableSamplerBinding(uint32_t binding,
                                                VkShaderStageFlags stages,
                                                std::vector<VkSampler> samplers);

            [[nodiscard]] VulkanDescriptorSetLayout Build() const;

        private:
            VkDevice m_Device;
            std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
            std::vector<std::vector<VkSampler>> m_ImmutableSamplers;
        };

    public:
        VulkanDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout);
        ~VulkanDescriptorSetLayout();

        VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout &) = delete;
        VulkanDescriptorSetLayout &operator=(const VulkanDescriptorSetLayout &) = delete;

        VulkanDescriptorSetLayout(VulkanDescriptorSetLayout &&other) noexcept;
        VulkanDescriptorSetLayout &operator=(VulkanDescriptorSetLayout &&other) noexcept;

        VkDescriptorSetLayout GetVkHandle() const { return m_Layout; }

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
    };

} // namespace ve
