#include "ShaderModule.hpp"
#include "Device.hpp"
#include "Validation.hpp"

#include <fstream>
#include <stdexcept>

namespace VE
{
    ShaderModule::ShaderModule(const Device &device, const std::string &filename)
        : ShaderModule(device, ReadFile(filename))
    {
    }

    ShaderModule::ShaderModule(const Device &device, const std::vector<char> &code)
        : m_Device(device)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        CheckVk(vkCreateShaderModule(m_Device.Handle(), &createInfo, nullptr, &m_ShaderModule), "create shader module!");
    }

    ShaderModule::~ShaderModule()
    {
        vkDestroyShaderModule(m_Device.Handle(), m_ShaderModule, nullptr);
    }

    VkPipelineShaderStageCreateInfo ShaderModule::CreateShaderStage(VkShaderStageFlagBits stage) const
    {
        VkPipelineShaderStageCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage = stage;
        createInfo.module = m_ShaderModule;
        createInfo.pName = "main";

        return createInfo;
    }

    std::vector<char> ShaderModule::ReadFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("failed to open file: {}" + filename);

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

        file.close();
        return buffer;
    }

}
