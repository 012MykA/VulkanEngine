#pragma once

#include "Debug/VulkanValidation.hpp"
#include "VulkanCommandPool.hpp"

#include <vulkan/vulkan.h>

#include <functional>

namespace ve
{
    class VulkanSingleTimeCommands
    {
    public:
        static void Submit(const std::function<void(VkCommandBuffer)> &action,
                           VkCommandPool commandPool, VkDevice device, VkQueue graphicsQueue)
        {
            VkCommandBufferAllocateInfo allocInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            VkCommandBuffer commandBuffer;
            VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
            CHECK_VK_RESULT(result);

            VkCommandBufferBeginInfo beginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            };

            result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
            CHECK_VK_RESULT(result);

            action(commandBuffer);

            result = vkEndCommandBuffer(commandBuffer);
            CHECK_VK_RESULT(result);

            VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer,
            };

            result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            CHECK_VK_RESULT(result);

            result = vkQueueWaitIdle(graphicsQueue);
            CHECK_VK_RESULT(result);

            vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        }
    };

} // namespace ve
