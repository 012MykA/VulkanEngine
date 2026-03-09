#include "VulkanShaderModule.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <fstream>
#include <stdexcept>

namespace ve
{
    VulkanShaderModule::VulkanShaderModule(VkDevice device, const std::filesystem::path &filename, const std::string &debugName)
        : VulkanShaderModule(device, ReadFile(filename), debugName)
    {
    }

    VulkanShaderModule::VulkanShaderModule(VkDevice device, const std::vector<char> &code, const std::string &debugName)
        : m_Device(device), m_DebugName(debugName)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
        createInfo.flags = 0;       // Optional
        createInfo.pNext = nullptr; // Optional

        VkResult result = vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_ShaderModule);
        CHECK_VK_RESULT(result);

        VE_CORE_TRACE("VulkanShaderModule ({0}) created", m_DebugName);
    }

    VulkanShaderModule::~VulkanShaderModule()
    {
        vkDestroyShaderModule(m_Device, m_ShaderModule, nullptr);
        VE_CORE_TRACE("VulkanShaderModule ({0}) destroyed", m_DebugName);
    }

    VkPipelineShaderStageCreateInfo VulkanShaderModule::CreateShaderStage(VkShaderStageFlagBits stage, const char *name) const
    {
        VkPipelineShaderStageCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage = stage;
        createInfo.module = m_ShaderModule;
        createInfo.pName = name;

        return createInfo;
    }

    std::vector<char> VulkanShaderModule::ReadFile(const std::filesystem::path &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("failed to open file: " + filename.string());

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
        file.close();

        return buffer;
    }

} // namespace ve
