#pragma once

#include "VulkanEngine/Core/Base.hpp"
#include "VulkanConfig.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanPipelineLayout.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanCommandPool.hpp"

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace ve
{
    class VulkanCore
    {
    public:
        VulkanCore();
        ~VulkanCore();

        void Init(const VulkanConfig &config, GLFWwindow *window);

        void DrawFrame();

    private:
        void CreateInstance(const VulkanConfig &config);
        void CreateDebugCallback(const VulkanConfig &config);
        void CreateSurface(GLFWwindow *window);
        void CreateDevice(const PhysicalDeviceRequirements &requirements);
        void CreateFramebuffers();
        void CreateCommandPool();
        void CreateCommandBuffer();
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void CreateSyncObjects();

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        Scope<VulkanPhysicalDevice> m_PhysicalDevice = nullptr;
        VkDevice m_Device = VK_NULL_HANDLE;
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
        Scope<VulkanSwapchain> m_Swapchain = nullptr;
        Scope<VulkanRenderPass> m_RenderPass = nullptr;
        Scope<VulkanPipelineLayout> m_PipelineLayout = nullptr;
        Scope<VulkanPipeline> m_Pipeline = nullptr;
        std::vector<Scope<VulkanFramebuffer>> m_Framebuffers;
        Scope<VulkanCommandPool> m_CommandPool = nullptr;
        VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
        VkSemaphore m_ImageAvailableSemaphore;
        VkSemaphore m_RenderFinishedSemaphore;
        VkFence m_InFlightFence;
    };

} // namespace ve
