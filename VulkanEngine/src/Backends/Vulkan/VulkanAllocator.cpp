#define VMA_IMPLEMENTATION
#include "VulkanAllocator.hpp"

#include "VulkanInstance.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanLogicalDevice.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    VulkanAllocator::VulkanAllocator(
        const VulkanInstance &instance,
        const VulkanPhysicalDevice &physicalDevice,
        const VulkanLogicalDevice &logicalDevice)
    {
        VmaVulkanFunctions vulkanFunctions{
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        };

        VmaAllocatorCreateInfo createInfo{
            .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
            .physicalDevice = physicalDevice.GetVkHandle(),
            .device = logicalDevice.GetVkHandle(),
            .pVulkanFunctions = &vulkanFunctions,
            .instance = instance.GetVkHandle(),
            .vulkanApiVersion = VK_API_VERSION_1_3,
        };

        VkResult result = vmaCreateAllocator(&createInfo, &m_Allocator);
        CHECK_VK_RESULT(result);
    }

    VulkanAllocator::~VulkanAllocator()
    {
        if (m_Allocator != nullptr)
        {
            vmaDestroyAllocator(m_Allocator);
            m_Allocator = nullptr;
        }
    }

    Allocation VulkanAllocator::AllocateBuffer(
        const VkBufferCreateInfo &bufferInfo,
        const AllocationDesc &allocDesc,
        VkBuffer &outBuffer) const
    {
        VmaAllocationCreateInfo vmaInfo = BuildVmaAllocInfo(allocDesc);

        Allocation allocation;
        VkResult result = vmaCreateBuffer(
            m_Allocator,
            &bufferInfo,
            &vmaInfo,
            &outBuffer,
            &allocation.handle,
            &allocation.info);
        CHECK_VK_RESULT(result);

        if (allocDesc.persistentMap)
            allocation.mappedPtr = allocation.info.pMappedData;

        return allocation;
    }

    void VulkanAllocator::FreeBuffer(VkBuffer buffer, const Allocation &allocation) const
    {
        vmaDestroyBuffer(m_Allocator, buffer, allocation.handle);
    }

    Allocation VulkanAllocator::AllocateImage(
        const VkImageCreateInfo &imageInfo,
        const AllocationDesc &allocDesc,
        VkImage &outImage) const
    {
        VmaAllocationCreateInfo vmaInfo = BuildVmaAllocInfo(allocDesc);

        Allocation allocation;
        VkResult result = vmaCreateImage(
            m_Allocator,
            &imageInfo,
            &vmaInfo,
            &outImage,
            &allocation.handle,
            &allocation.info);
        CHECK_VK_RESULT(result);

        return allocation;
    }

    void VulkanAllocator::FreeImage(VkImage image, const Allocation &allocation) const
    {
        vmaDestroyImage(m_Allocator, image, allocation.handle);
    }

    void *VulkanAllocator::MapMemory(const Allocation &allocation) const
    {
        void *ptr = nullptr;
        VkResult result = vmaMapMemory(m_Allocator, allocation.handle, &ptr);
        CHECK_VK_RESULT(result);
        return ptr;
    }

    void VulkanAllocator::UnmapMemory(const Allocation &allocation) const
    {
        vmaUnmapMemory(m_Allocator, allocation.handle);
    }

    void VulkanAllocator::FlushAllocation(const Allocation &allocation, VkDeviceSize offset, VkDeviceSize size) const
    {
        vmaFlushAllocation(m_Allocator, allocation.handle, offset, size);
    }

    void VulkanAllocator::InvalidateAllocation(const Allocation &allocation, VkDeviceSize offset, VkDeviceSize size) const
    {
        vmaInvalidateAllocation(m_Allocator, allocation.handle, offset, size);
    }

    VmaAllocationCreateInfo VulkanAllocator::BuildVmaAllocInfo(const AllocationDesc &desc) const
    {
        VmaAllocationCreateInfo info{};
        info.requiredFlags = desc.requiredFlags;
        info.preferredFlags = desc.preferredFlags;

        if (desc.cpuOnly)
        {
            info.usage = VMA_MEMORY_USAGE_AUTO;
            info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        }
        else if (desc.gpuOnly)
        {
            info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        }
        else if (desc.cpuToGpu)
        {
            info.usage = VMA_MEMORY_USAGE_AUTO;
            info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                         VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
        }
        else
        {
            info.usage = VMA_MEMORY_USAGE_AUTO;
        }

        if (desc.persistentMap)
            info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

        return info;
    }

    AllocatorStats VulkanAllocator::GetStats() const
    {
        VmaTotalStatistics stats{};
        vmaCalculateStatistics(m_Allocator, &stats);

        AllocatorStats allocatorStats{
            .allocationCount = stats.total.statistics.allocationCount,
            .blockCount = stats.total.statistics.blockCount,
            .usedBytes = stats.total.statistics.allocationBytes,
            .totalBytes = stats.total.statistics.blockBytes,
        };

        return allocatorStats;
    }

} // namespace ve
