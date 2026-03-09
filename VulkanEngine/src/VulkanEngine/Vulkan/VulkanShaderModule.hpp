#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <filesystem>
#include <string>

namespace ve
{    
    class VulkanShaderModule
    {
    public:
        VulkanShaderModule(VkDevice device, const std::filesystem::path &filename);
        VulkanShaderModule(VkDevice device, const std::vector<char> &code);
        ~VulkanShaderModule();

        VkPipelineShaderStageCreateInfo CreateShaderStage(VkShaderStageFlagBits stage, const std::string &name = "main") const;

    private:
        static std::vector<char> ReadFile(const std::filesystem::path &filename);

    private:
        VkDevice m_Device;
        VkShaderModule m_ShaderModule = VK_NULL_HANDLE;
    };

} // namespace ve
