#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace VE
{
    class Device;

    class ShaderModule
    {
    public:
        ShaderModule(const Device &device, const std::string &filename);
        ShaderModule(const Device &device, const std::vector<char> &code);
        ~ShaderModule();

        VkPipelineShaderStageCreateInfo CreateShaderStage(VkShaderStageFlagBits stage) const;

    private:
        static std::vector<char> ReadFile(const std::string &filename);

    private:
        const Device &m_Device;
        VkShaderModule m_ShaderModule;
    };

}
