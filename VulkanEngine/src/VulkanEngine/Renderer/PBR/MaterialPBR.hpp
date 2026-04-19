#pragma once

#include "Backends/Vulkan/VulkanDescriptor.hpp"
#include "Backends/Vulkan/VulkanBuffer.hpp"
#include "VulkanEngine/Renderer/Texture.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <string>
#include <memory>

namespace ve
{
    class VulkanAllocator;
    class VulkanLogicalDevice;
    class VulkanImmediateSubmit;

    struct alignas(16) MaterialPBRData
    {
        // --- Factors ---
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        glm::vec3 emissiveFactor = glm::vec3(0.0f);
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        float alphaCutoff = 0.5f;
        float normalScale = 1.0f;
        float occlusionStrength = 1.0f;

        // --- UV Transform ---
        glm::vec2 uvScale = glm::vec2(1.0f);
        glm::vec2 uvOffset = glm::vec2(0.0f);

        // --- Texture Indices ---
        int baseColorTextureIdx = -1;
        int emissiveTextureIdx = -1;
        int metallicTextureIdx = -1; // B=metallic, G=roughness
        int roughnessTextureIdx = -1;
        int normalTextureIdx = -1;
        int occlusionTextureIdx = -1;

        // --- Modes ---
        int alphaMode = 0; // 0: OPAQUE, 1: MASK, 2: BLEND
        float _padding[1];
    };

    class MaterialPBR
    {
    public:
        MaterialPBR() = default;
        ~MaterialPBR() = default;

        MaterialPBR(const MaterialPBR &) = delete;
        MaterialPBR &operator=(const MaterialPBR &) = delete;

        static std::shared_ptr<MaterialPBR> Load(const std::string &path,
                                                 const VulkanAllocator &allocator,
                                                 const VulkanLogicalDevice &logicalDevice,
                                                 const VulkanImmediateSubmit &upload,
                                                 bool generateMips = true);

    public:
        void Build(const VulkanAllocator &allocator,
                   const VulkanLogicalDevice &logicalDevice,
                   const VulkanDescriptorPool &pool,
                   const VulkanDescriptorSetLayout &layout,
                   const Texture &defaultWhite,
                   const Texture &defaultNormalMap);

        void UpdateGPU();

    public:
        static VulkanDescriptorSetLayout CreateLayout(const VulkanLogicalDevice &device);
        VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }

        const MaterialPBRData &GetData() const { return m_Data; }
        bool IsBuilt() const { return m_DescriptorSet != VK_NULL_HANDLE; }

    public:
        void SetName(const std::string &name);
        void SetBaseColorFactor(const glm::vec4 &color);
        void SetEmissiveFactor(const glm::vec3 &color);
        void SetMetallicFactor(float v);
        void SetRoughnessFactor(float v);

        void SetUVScale(const glm::vec2 &scale);
        void SetUVOffset(const glm::vec2 &offset);

        void SetBaseColorMap(std::shared_ptr<Texture> tex);
        void SetEmissiveMap(std::shared_ptr<Texture> tex);
        void SetMetallicMap(std::shared_ptr<Texture> tex);
        void SetRoughnessMap(std::shared_ptr<Texture> tex);
        void SetNormalMap(std::shared_ptr<Texture> tex);
        void SetOcclusionMap(std::shared_ptr<Texture> tex);

    private:
        std::string m_Name = "Unnamed";
        MaterialPBRData m_Data{};

        std::shared_ptr<Texture> m_BaseColorMap;
        std::shared_ptr<Texture> m_EmissiveMap;
        std::shared_ptr<Texture> m_MetallicMap;
        std::shared_ptr<Texture> m_RoughnessMap;
        std::shared_ptr<Texture> m_NormalMap;
        std::shared_ptr<Texture> m_OcclusionMap;

        std::unique_ptr<VulkanBuffer> m_UBO;
        VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
    };

} // namespace ve
