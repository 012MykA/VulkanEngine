#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <span>
#include <cstdint>

namespace ve
{
    class VulkanLogicalDevice;
    
    class VulkanShader
    {
    public:
        VulkanShader(const VulkanLogicalDevice &device, const std::string &spvPath);
        VulkanShader(const VulkanLogicalDevice &device, std::span<const uint32_t> spvCode);
        ~VulkanShader();

        VulkanShader(const VulkanShader &) = delete;
        VulkanShader &operator=(const VulkanShader &) = delete;

        VkShaderModule GetVkHandle() const { return m_Module; }

    private:
        static std::vector<uint32_t> LoadSpv(const std::string &path);

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VkShaderModule m_Module = VK_NULL_HANDLE;
    };

} // namespace ve
