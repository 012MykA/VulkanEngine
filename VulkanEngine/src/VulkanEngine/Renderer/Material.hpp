#pragma once

#include <vulkan/vulkan.h>

#include "VulkanEngine/Vulkan/VulkanDescriptor.hpp"
#include "VulkanEngine/Vulkan/VulkanBuffer.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <string>

namespace ve
{
    class VulkanAllocator;
    class VulkanLogicalDevice;

    struct MaterialData
    {
        alignas(16) glm::vec4 ambientColor = glm::vec4(0.24725f, 0.1995f, 0.0745f, 0.0f);
        alignas(16) glm::vec4 diffuseColor = glm::vec4(0.75164f, 0.60648f, 0.22648f, 0.0f);
        // xyz = specular color, w = shininess
        alignas(16) glm::vec4 specularColor = glm::vec4(0.628281f, 0.555802f, 0.366065f, 51.2f);
    };

    class Material
    {
    public:
        Material() = default;
        ~Material();

        Material(const Material &) = delete;
        Material &operator=(const Material &) = delete;

    public: // GPU resources
        void Build(
            const VulkanAllocator &allocator,
            const VulkanLogicalDevice &device,
            const VulkanDescriptorPool &pool,
            const VulkanDescriptorSetLayout &layout);

        void UpdateGPU();

    public: // Setters
        void SetName(const std::string &name) { m_Name = name; }
        void SetAmbient(glm::vec3 color) { m_Data.ambientColor = glm::vec4(color, 1.0f); }
        void SetDiffuse(glm::vec3 color) { m_Data.diffuseColor = glm::vec4(color, 1.0f); }
        void SetSpecular(glm::vec3 color, float shine) { m_Data.specularColor = glm::vec4(color, shine); }

        // Getters
        VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }
        const MaterialData &GetData() const { return m_Data; }
        bool IsBuilt() const { return m_UBO != nullptr; }

    private:
        const VulkanLogicalDevice *m_Device;

        std::string m_Name = "Unnamed";
        MaterialData m_Data{};

        std::unique_ptr<VulkanBuffer> m_UBO;
        VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
    };

} // namespace ve
