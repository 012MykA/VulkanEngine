#pragma once

#include <vulkan/vulkan.h>

#include "Texture.hpp"
#include "VulkanEngine/Vulkan/VulkanDescriptor.hpp"
#include "VulkanEngine/Vulkan/VulkanBuffer.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <string>

namespace ve
{
    class VulkanAllocator;
    class VulkanLogicalDevice;

    enum class AlphaMode
    {
        Opaque,
        Mask,
        Blend
    };

    struct alignas(16) MaterialUBO
    {
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        glm::vec4 emissiveFactor = glm::vec4(0.0f);

        float metallic = 0.0f;
        float roughness = 1.0f;
        float normalScale = 1.0f;
        float occlusionStrength = 1.0f;

        float alphaCutoff = 0.5f;
        float _pad0 = 0.0f;
        float _pad1 = 0.0f;
        float _pad2 = 0.0f;

        // Flags
        int hasAlbedoMap = 0;
        int hasNormalMap = 0;
        int hasMetallicRoughnessMap = 0;
        int hasEmissiveMap = 0;

        int hasOcclusionMap = 0;
        int alphaMode = 0;
        int _pad3 = 0;
        int _pad4 = 0;
    };

    namespace MaterialBindings
    {
        constexpr uint32_t UBO = 0;
        constexpr uint32_t Albedo = 1;
        constexpr uint32_t Normal = 2;
        constexpr uint32_t MetallicRoughness = 3;
        constexpr uint32_t Emissive = 4;
        constexpr uint32_t Occlusion = 5;
    }

    class Material
    {
    public:
        Material() = default;
        ~Material() = default;

        Material(const Material &) = delete;
        Material &operator=(const Material &) = delete;

        static VulkanDescriptorSetLayout CreateLayout(const VulkanLogicalDevice &logicalDevice);

        void Build(
            const VulkanAllocator &allocator,
            const VulkanLogicalDevice &logicalDevice,
            const VulkanDescriptorPool &pool,
            const VulkanDescriptorSetLayout &layout,
            const Texture &defaultWhite,
            const Texture &defaultNormal);

        void UpdateUBO();

    public:
        // Setters
        void SetName(std::string name) { m_Name = std::move(name); }

        void SetAlbedoMap(std::shared_ptr<Texture> tex);
        void SetNormalMap(std::shared_ptr<Texture> tex);
        void SetMetallicRoughnessMap(std::shared_ptr<Texture> tex);
        void SetEmissiveMap(std::shared_ptr<Texture> tex);
        void SetOcclusionMap(std::shared_ptr<Texture> tex);

        void SetBaseColorFactor(glm::vec4 v) { m_UBO.baseColorFactor = v; }
        void SetEmissiveFactor(glm::vec3 v) { m_UBO.emissiveFactor = glm::vec4(v, 0); }
        void SetEmissiveIntensity(float i) { m_UBO.emissiveFactor.w = i; }
        void SetMetallic(float v) { m_UBO.metallic = v; }
        void SetRoughness(float v) { m_UBO.roughness = v; }
        void SetNormalScale(float v) { m_UBO.normalScale = v; }
        void SetOcclusionStrength(float v) { m_UBO.occlusionStrength = v; }
        void SetAlphaCutoff(float v) { m_UBO.alphaCutoff = v; }
        void SetAlphaMode(AlphaMode mode);
        void SetDoubleSided(bool v) { m_DoubleSided = v; }

        // Getters
        const std::string &GetName() const { return m_Name; }
        AlphaMode GetAlphaMode() const { return m_AlphaMode; }
        bool IsDoubleSided() const { return m_DoubleSided; }
        bool IsEmissive() const { return glm::length(glm::vec3(m_UBO.emissiveFactor)) > 0.0f || m_UBO.hasEmissiveMap; }

        VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }
        const MaterialUBO &GetUBO() const { return m_UBO; }

    private:
        std::string m_Name;
        AlphaMode m_AlphaMode = AlphaMode::Opaque;
        bool m_DoubleSided = false;

        // Texures
        std::shared_ptr<Texture> m_AlbedoMap;
        std::shared_ptr<Texture> m_NormalMap;
        std::shared_ptr<Texture> m_MetallicRoughnessMap;
        std::shared_ptr<Texture> m_EmissiveMap;
        std::shared_ptr<Texture> m_OcclusionMap;

        // GPU Data
        MaterialUBO m_UBO{};
        std::unique_ptr<VulkanBuffer> m_UBOBuffer;
        VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
    };

} // namespace ve
