#pragma once

// Disable VMA warnings
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnullability-extension"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>

#include <vulkan/vulkan.h>

namespace ve
{
    class VulkanInstance;
    class VulkanPhysicalDevice;
    class VulkanLogicalDevice;

    struct AllocationDesc
    {
        VkMemoryPropertyFlags requiredFlags = 0;
        VkMemoryPropertyFlags preferredFlags = 0;

        bool cpuOnly = false;
        bool gpuOnly = false;
        bool cpuToGpu = false;

        bool persistentMap = false;
    };

    struct Allocation
    {
        VmaAllocation handle = VK_NULL_HANDLE;
        VmaAllocationInfo info = {};
        void *mappedPtr = nullptr;
    };

    class VulkanAllocator
    {
    public:
        VulkanAllocator(
            const VulkanInstance &instance,
            const VulkanPhysicalDevice &physicalDevice,
            const VulkanLogicalDevice &logicalDevice);

        ~VulkanAllocator();

        VulkanAllocator(const VulkanAllocator &) = delete;
        VulkanAllocator &operator=(const VulkanAllocator &) = delete;

    public:
        // Buffer
        [[nodiscard]] Allocation AllocateBuffer(
            const VkBufferCreateInfo &bufferInfo,
            const AllocationDesc &allocDesc,
            VkBuffer &outBuffer) const;

        void FreeBuffer(VkBuffer buffer, const Allocation &allocation) const;

        // Image
        [[nodiscard]] Allocation AllocateImage(
            const VkImageCreateInfo &imageInfo,
            const AllocationDesc &allocDesc,
            VkImage &outImage) const;

        void FreeImage(VkImage image, const Allocation &allocation) const;

        // Map
        void *MapMemory(const Allocation &allocation) const;
        void UnmapMemory(const Allocation &allocation) const;

        void FlushAllocation(const Allocation &allocation, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;
        void InvalidateAllocation(const Allocation &allocation, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

        void PrintStats() const;

        VmaAllocator GetVmaHandle() const { return m_Allocator; }

    private:
        VmaAllocationCreateInfo BuildVmaAllocInfo(const AllocationDesc &desc) const;

    private:
        VmaAllocator m_Allocator = VK_NULL_HANDLE;
    };

} // namespace ve
