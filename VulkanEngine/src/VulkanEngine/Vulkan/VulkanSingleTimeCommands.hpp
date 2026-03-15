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
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
            CHECK_VK_RESULT(result);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
            CHECK_VK_RESULT(result);

            action(commandBuffer);

            result = vkEndCommandBuffer(commandBuffer);
            CHECK_VK_RESULT(result);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            CHECK_VK_RESULT(result);

            result = vkQueueWaitIdle(graphicsQueue);
            CHECK_VK_RESULT(result);

            vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        }
    };

} // namespace ve
