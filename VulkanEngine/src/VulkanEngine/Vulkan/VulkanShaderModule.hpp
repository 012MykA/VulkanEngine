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
        VulkanShaderModule(VkDevice device, const std::filesystem::path &filename, const std::string &debugName = "Unnamed");
        VulkanShaderModule(VkDevice device, const std::vector<char> &code, const std::string &debugName = "Unnamed");
        ~VulkanShaderModule();

        VkPipelineShaderStageCreateInfo CreateShaderStage(VkShaderStageFlagBits stage, const char *name = "main") const;

    private:
        static std::vector<char> ReadFile(const std::filesystem::path &filename);

    private:
        VkDevice m_Device;
        VkShaderModule m_ShaderModule = VK_NULL_HANDLE;

        std::string m_DebugName;
    };

} // namespace ve
