#pragma once

#include "Backends/Vulkan/VulkanBuffer.hpp"
#include "VulkanEngine/Renderer/Texture.hpp"

#include <cstdint>
#include <string>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace ve
{
    class VulkanAllocator;
    class VulkanDescriptorPool;
    class VulkanDescriptorSetLayout;
    class VulkanLogicalDevice;

    enum class AlphaMode : uint32_t
    {
        Opaque = 0,
        Mask = 1,
        Blend = 2
    };

    struct alignas(16) MaterialPBRData
    {
        glm::vec4 baseColorFactor = glm::vec4(1.0f);

        glm::vec3 emissiveColorFactor = glm::vec3(0.0f);
        float emissiveStrength = 1.0f;

        float alphaCutoff = 0.5f;
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        float normalScale = 1.0f;

        float occlusionStrength = 1.0f;
        uint32_t alphaMode = 0;   // 0: Opaque, 1: Mask, 2: Blend
        uint32_t doubleSided = 0; // 0: false, 1: true
        uint32_t hasBaseColorMap = 0;

        uint32_t hasNormalMap = 0;
        uint32_t hasAoMetallicRoughnessMap = 0;
        uint32_t hasEmissiveMap = 0;
        float _pad[1];
    };

    class MaterialPBR
    {
    private:
        struct Binding
        {
            static constexpr uint32_t BufferInfo = 0;
            static constexpr uint32_t BaseColor = 1;
            static constexpr uint32_t Emissive = 2;
            static constexpr uint32_t MetallicRoughness = 3;
            static constexpr uint32_t Normal = 4;
        };

    public:
        MaterialPBR() = default;
        ~MaterialPBR() = default;

        MaterialPBR(const MaterialPBR &) = delete;
        MaterialPBR &operator=(const MaterialPBR &) = delete;

        static VulkanDescriptorSetLayout CreateLayout(const VulkanLogicalDevice &device);

    public:
        void Upload(const VulkanAllocator &allocator,
                    const VulkanDescriptorPool &pool,
                    const VulkanDescriptorSetLayout &layout,
                    const VulkanLogicalDevice &logicalDevice,
                    const Texture &defaultWhite,
                    const Texture &defaultNormal);

        void UpdateGPU();
        void Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t setIndex) const;

        // --- Setters ---

        void SetName(const std::string &name) { m_Name = name; }

        void SetBaseColorFactor(const glm::vec4 &factor) { m_Data.baseColorFactor = factor; }
        void SetEmissiveColorFactor(const glm::vec3 &factor) { m_Data.emissiveColorFactor = factor; }
        void SetEmissiveStrength(float strength) { m_Data.emissiveStrength = strength; }
        void SetMetallicFactor(float factor) { m_Data.metallicFactor = factor; }
        void SetRoughnessFactor(float factor) { m_Data.roughnessFactor = factor; }
        void SetNormalScale(float scale) { m_Data.normalScale = scale; }
        void SetOcclusionStrength(float strength) { m_Data.occlusionStrength = strength; }
        void SetAlphaMode(AlphaMode mode) { m_Data.alphaMode = static_cast<uint32_t>(mode); }
        void SetAlphaCutoff(float cutoff) { m_Data.alphaCutoff = cutoff; }
        void SetDoubleSided(bool doubleSided) { m_Data.doubleSided = doubleSided ? 1 : 0; }

        // clang-format off
        void SetBaseColorMap(std::shared_ptr<Texture> tex) { m_BaseColorMap = std::move(tex); m_Data.hasBaseColorMap = 1;}
        void SetNormalMap(std::shared_ptr<Texture> tex) { m_NormalMap = std::move(tex); m_Data.hasNormalMap = 1;}
        void SetAoMetallicRoughnessMap(std::shared_ptr<Texture> tex) { m_AoMetallicRoughnessMap = std::move(tex); m_Data.hasAoMetallicRoughnessMap = 1;}
        void SetEmissiveMap(std::shared_ptr<Texture> tex) { m_EmissiveMap = std::move(tex); m_Data.hasEmissiveMap = 1;}
        // clang-format on

        // --- Getters ---

        const glm::vec3 &GetEmissiveColorFactor() const { return m_Data.emissiveColorFactor; }
        float GetEmissiveStrength() const { return m_Data.emissiveStrength; }

        VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }
        const std::string &GetName() const { return m_Name; }
        AlphaMode GetAlphaMode() const { return static_cast<AlphaMode>(m_Data.alphaMode); }
        bool IsDoubleSided() const { return m_Data.doubleSided == 1; }
        bool IsEmissive() const { return glm::any(glm::greaterThan(m_Data.emissiveColorFactor, glm::vec3(0.0f))); }

        const std::shared_ptr<Texture> &GetBaseColorMap() const { return m_BaseColorMap; }
        const std::shared_ptr<Texture> &GetNormalMap() const { return m_NormalMap; }
        const std::shared_ptr<Texture> &GetAoMetallicRoughnessMap() const { return m_AoMetallicRoughnessMap; }
        const std::shared_ptr<Texture> &GetEmissiveMap() const { return m_EmissiveMap; }

    private:
        std::string m_Name;
        MaterialPBRData m_Data;

        std::shared_ptr<Texture> m_BaseColorMap;
        std::shared_ptr<Texture> m_NormalMap;
        std::shared_ptr<Texture> m_AoMetallicRoughnessMap;
        std::shared_ptr<Texture> m_EmissiveMap;

        std::unique_ptr<VulkanBuffer> m_UBO;
        VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
    };

} // namespace ve
