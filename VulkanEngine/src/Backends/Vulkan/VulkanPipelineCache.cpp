#include "VulkanPipelineCache.hpp"
#include "VulkanLogicalDevice.hpp"
#include "Debug/VulkanValidation.hpp"

#include <fstream>
#include <filesystem>

namespace ve
{
    VulkanPipelineCache::VulkanPipelineCache(VkDevice device, VkPipelineCache cache)
        : m_Device(device), m_Cache(cache)
    {
    }

    VulkanPipelineCache::VulkanPipelineCache(const VulkanLogicalDevice &device, const std::string &cachePath)
        : VulkanPipelineCache(device, LoadCacheBinary(cachePath))
    {
    }

    VulkanPipelineCache::VulkanPipelineCache(const VulkanLogicalDevice &device, const std::vector<char> &cacheData)
        : m_Device(device.GetVkHandle())
    {
        VkPipelineCacheCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            .initialDataSize = cacheData.size(),
            .pInitialData = cacheData.data(),
        };

        CHECK_VK_RESULT(vkCreatePipelineCache(m_Device, &createInfo, nullptr, &m_Cache));
    }

    VulkanPipelineCache::~VulkanPipelineCache()
    {
        if (m_Cache != VK_NULL_HANDLE)
            vkDestroyPipelineCache(m_Device, m_Cache, nullptr);
    }

    void VulkanPipelineCache::SaveCacheToFile(const std::string &path) const
    {
        size_t cacheSize = 0;
        CHECK_VK_RESULT(vkGetPipelineCacheData(m_Device, m_Cache, &cacheSize, nullptr));

        std::vector<char> cacheData(cacheSize);
        CHECK_VK_RESULT(vkGetPipelineCacheData(m_Device, m_Cache, &cacheSize, cacheData.data()));

        std::error_code ec;
        std::filesystem::path fsPath(path);
        if (fsPath.has_parent_path())
            std::filesystem::create_directories(fsPath.parent_path(), ec);

        std::ofstream file(path, std::ios::out | std::ios::binary);
        if (file.is_open())
        {
            file.write(cacheData.data(), static_cast<std::streamsize>(cacheSize));
            file.close();
        }
        else
        {
            VE_CORE_ERROR("Failed to open file for writing pipeline cache: " + path);
        }
    }

    std::vector<char> VulkanPipelineCache::LoadCacheBinary(const std::string &path)
    {
        if (!std::filesystem::exists(path))
            return {};

        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            VE_CORE_WARN("Failed to open pipeline cache file: " + path);
            return {};
        }

        std::streamsize size = file.tellg();
        if (size <= 0)
            return {};

        file.seekg(0);
        std::vector<char> data(static_cast<size_t>(size));
        if (!file.read(data.data(), size))
        {
            VE_CORE_WARN("Failed to read pipeline cache file: " + path);
            return {};
        }

        return data;
    }

    bool VulkanPipelineCache::IsCacheDataValid(const std::vector<char> &cacheData,
                                               const VkPhysicalDeviceProperties &deviceProps)
    {
        // typedef struct VkPipelineCacheHeaderVersionOne
        // {
        //     uint32_t headerSize;
        //     VkPipelineCacheHeaderVersion headerVersion;
        //     uint32_t vendorID;
        //     uint32_t deviceID;
        //     uint8_t pipelineCacheUUID[VK_UUID_SIZE];
        // } VkPipelineCacheHeaderVersionOne;

        constexpr size_t headerSize = 4 + 4 + 4 + 4 + VK_UUID_SIZE;

        if (cacheData.size() < headerSize)
            return false;

        uint32_t headerVersion = 0;
        uint32_t vendorID = 0;
        uint32_t deviceID = 0;

        std::memcpy(&headerVersion, cacheData.data() + 4, sizeof(uint32_t));
        std::memcpy(&vendorID, cacheData.data() + 8, sizeof(uint32_t));
        std::memcpy(&deviceID, cacheData.data() + 12, sizeof(uint32_t));

        if (headerVersion != VK_PIPELINE_CACHE_HEADER_VERSION_ONE)
            return false;
        if (vendorID != deviceProps.vendorID || deviceID != deviceProps.deviceID)
            return false;
        if (std::memcmp(cacheData.data() + 16, deviceProps.pipelineCacheUUID, VK_UUID_SIZE) != 0)
            return false;

        return true;
    }

} // namespace ve
