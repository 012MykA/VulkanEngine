#include "VulkanShader.hpp"
#include "VulkanLogicalDevice.hpp"
#include "Debug/VulkanValidation.hpp"

#include <fstream>

namespace ve
{
    VulkanShader::VulkanShader(const VulkanLogicalDevice &device, const std::string &spvPath)
        : VulkanShader(device, LoadSpv(spvPath))
    {
    }

    VulkanShader::VulkanShader(const VulkanLogicalDevice &device, std::span<const uint32_t> spvCode)
        : m_Device(device.GetVkHandle())
    {
        VkShaderModuleCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = spvCode.size() * sizeof(uint32_t),
            .pCode = spvCode.data(),
        };

        VkResult result = vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_Module);
        CHECK_VK_RESULT(result);
    }

    VulkanShader::~VulkanShader()
    {
        if (m_Module != VK_NULL_HANDLE)
            vkDestroyShaderModule(m_Device, m_Module, nullptr);
    }

    std::vector<uint32_t> VulkanShader::LoadSpv(const std::string &path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
            throw std::runtime_error("Failed to open shader: " + path);

        size_t size = static_cast<size_t>(file.tellg());
        if (size % 4 != 0)
            throw std::runtime_error("SPIR-V size not aligned to 4 bytes: " + path);

        file.seekg(0);
        std::vector<uint32_t> code(size / 4);
        file.read(reinterpret_cast<char *>(code.data()), static_cast<std::streamsize>(size));
        return code;
    }

} // namespace ve
