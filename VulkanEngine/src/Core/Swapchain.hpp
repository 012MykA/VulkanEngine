#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace VE
{
    class Device;
    class Surface;
    class Window;

    class Swapchain final
    {
    public:
        Swapchain(const Device &device, const Surface &surface, const Window &window);
        ~Swapchain();

        VkSwapchainKHR Handle() const { return m_Swapchain; }

    private:
        void CreateSwapchain(const Window &window);

        VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) const;
        VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR> &modes) const;

        VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR &capabilities, const Window &window) const;

    private:
        const Device &m_Device;
        const Surface &m_Surface;

        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

        VkFormat m_ImageFormat{};
        VkExtent2D m_Extent{};
    };

}
