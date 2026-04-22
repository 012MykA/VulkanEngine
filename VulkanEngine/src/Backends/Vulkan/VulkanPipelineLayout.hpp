#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace ve
{
    class VulkanLogicalDevice;

    class VulkanPipelineLayout
    {
    public:
        class Builder
        {
        public:
            explicit Builder(const VulkanLogicalDevice &logicalDevice);

            Builder &AddDescriptorSetLayout(VkDescriptorSetLayout layout);

            template <typename T>
            Builder &AddPushConstantRange(VkShaderStageFlags stages, uint32_t offset = 0)
            {
                m_PushConstants.push_back(VkPushConstantRange{
                    .stageFlags = stages,
                    .offset = offset,
                    .size = sizeof(T),
                });
                return *this;
            }

            [[nodiscard]] VulkanPipelineLayout Build() const;

        private:
            VkDevice m_Device = VK_NULL_HANDLE;
            std::vector<VkDescriptorSetLayout> m_SetLayouts;
            std::vector<VkPushConstantRange> m_PushConstants;
        };

    public:
        VulkanPipelineLayout(VkDevice device, VkPipelineLayout layout);
        ~VulkanPipelineLayout();

        VulkanPipelineLayout(const VulkanPipelineLayout &) = delete;
        VulkanPipelineLayout &operator=(const VulkanPipelineLayout &) = delete;

        VulkanPipelineLayout(VulkanPipelineLayout &&other) noexcept;
        VulkanPipelineLayout &operator=(VulkanPipelineLayout &&other) noexcept;

        VkPipelineLayout GetVkHandle() const { return m_Layout; }

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VkPipelineLayout m_Layout = VK_NULL_HANDLE;
    };

} // namespace ve
