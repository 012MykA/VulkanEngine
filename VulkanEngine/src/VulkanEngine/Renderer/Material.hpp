#pragma once

#include <vulkan/vulkan.h>

#include "VulkanEngine/Vulkan/VulkanDescriptor.hpp"
#include "VulkanEngine/Vulkan/VulkanBuffer.hpp"
#include "Texture.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <string>

namespace ve
{
    class VulkanAllocator;
    class VulkanLogicalDevice;

    struct MaterialData
    {
        alignas(16) glm::vec4 ambientColor = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
        alignas(16) glm::vec4 diffuseColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);   // a - unused
        alignas(16) glm::vec4 specularColor = glm::vec4(0.5f, 0.5f, 0.5f, 32.0f); // rgb = specular color, a = shininess

        alignas(4) int hasBaseColorTexture = 0;
        alignas(4) float _pad0 = 0, _pad1 = 0, _pad2 = 0;
    };

    class Material
    {
    public:
        Material() = default;
        ~Material() = default;

        Material(const Material &) = delete;
        Material &operator=(const Material &) = delete;

    public: // GPU resources
        void Build(const VulkanAllocator &allocator,
                   const VulkanLogicalDevice &device,
                   const VulkanDescriptorPool &pool,
                   const VulkanDescriptorSetLayout &layout,
                   const Texture &defaultWhite);

        void UpdateGPU();

    public: // Setters
        void SetName(const std::string &name) { m_Name = name; }
        void SetAmbient(glm::vec3 color) { m_Data.ambientColor = glm::vec4(color, 1.0f); }
        void SetDiffuse(glm::vec3 color) { m_Data.diffuseColor = glm::vec4(color, 1.0f); }
        void SetSpecular(glm::vec3 color) { m_Data.specularColor = glm::vec4(color, m_Data.specularColor.a); }
        void SetShininess(float shininess) { m_Data.specularColor.a = shininess; }
        void SetBaseColorTexture(std::shared_ptr<Texture> texture) 
        { 
            m_BaseColorTexture = std::move(texture); 
            m_Data.hasBaseColorTexture = 1;
        }

        // Getters
        VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }
        const MaterialData &GetData() const { return m_Data; }
        bool IsBuilt() const { return m_UBO != nullptr; }

    private:
        std::string m_Name = "Default";
        MaterialData m_Data{};

        std::shared_ptr<Texture> m_BaseColorTexture;

        std::unique_ptr<VulkanBuffer> m_UBO;
        VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
    };

} // namespace ve
