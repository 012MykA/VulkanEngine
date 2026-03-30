#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>
#include <cstdint>

namespace ve
{
    class VulkanLogicalDevice;
    class VulkanSwapchain;
    class VulkanRenderPass;

    class VulkanFramebuffers
    {
    public:
        VulkanFramebuffers(const VulkanLogicalDevice &logicalDevice,
                           const VulkanSwapchain &swapchain,
                           const VulkanRenderPass &renderPass);
        ~VulkanFramebuffers();

        VulkanFramebuffers(const VulkanFramebuffers &) = delete;
        VulkanFramebuffers &operator=(const VulkanFramebuffers &) = delete;

        void Recreate(const VulkanSwapchain &swapchain, const VulkanRenderPass &renderPass);

    public:
        // Getters
        VkFramebuffer GetFramebuffer(uint32_t imageIndex) const { return m_Framebuffers[imageIndex]; }
        uint32_t GetCount() const { return static_cast<uint32_t>(m_Framebuffers.size()); }

    private:
        void Create(const VulkanSwapchain &swapchain, const VulkanRenderPass &renderPass);
        void Destroy();

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> m_Framebuffers;
    };

} // namespace ve
