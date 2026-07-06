#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace ve
{
    class VulkanLogicalDevice;

    class VulkanPipelineCache
    {
    public:
        // Takes ownership of an already created VkPipelineCache descriptor
        VulkanPipelineCache(VkDevice device, VkPipelineCache cache);

        VulkanPipelineCache(const VulkanLogicalDevice &device, const std::string &cachePath);
        VulkanPipelineCache(const VulkanLogicalDevice &device, const std::vector<char> &cacheData);
        ~VulkanPipelineCache();

        VulkanPipelineCache(const VulkanPipelineCache &) = delete;
        VulkanPipelineCache &operator=(const VulkanPipelineCache &) = delete;

        VkPipelineCache GetVkHandle() const { return m_Cache; }

        void SaveCacheToFile(const std::string &path) const;

        static std::vector<char> LoadCacheBinary(const std::string &path);

        static bool IsCacheDataValid(const std::vector<char> &cacheData,
                                     const VkPhysicalDeviceProperties &deviceProps);

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VkPipelineCache m_Cache = VK_NULL_HANDLE;
    };

} // namespace ve
