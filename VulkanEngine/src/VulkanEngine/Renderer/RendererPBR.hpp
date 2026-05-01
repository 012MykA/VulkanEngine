#pragma once

#include "Backends/Vulkan/VulkanInstance.hpp"
#include "Backends/Vulkan/VulkanSurface.hpp"
#include "Backends/Vulkan/VulkanPhysicalDevice.hpp"
#include "Backends/Vulkan/VulkanLogicalDevice.hpp"
#include "Backends/Vulkan/VulkanAllocator.hpp"
#include "Backends/Vulkan/VulkanSwapchain.hpp"
#include "Backends/Vulkan/VulkanDepthBuffer.hpp"
#include "Backends/Vulkan/VulkanRenderPass.hpp"
#include "Backends/Vulkan/VulkanFramebuffers.hpp"
#include "Backends/Vulkan/VulkanCommandPool.hpp"
#include "Backends/Vulkan/VulkanImmediateSubmit.hpp"
#include "Backends/Vulkan/VulkanDescriptor.hpp"
#include "Backends/Vulkan/VulkanPipelineLayout.hpp"
#include "Backends/Vulkan/VulkanPipeline.hpp"
#include "Backends/Vulkan/VulkanFrameManager.hpp"

#include "VulkanEngine/Renderer/Mesh.hpp"
#include "VulkanEngine/Renderer/MaterialPBR.hpp"
#include "VulkanEngine/Renderer/RenderSettings.hpp"

#include <glm/glm.hpp>

#include <memory>

#define MAX_LIGHTS 10

namespace ve
{
    class Window;
    class Camera;

    struct PointLight
    {
        alignas(16) glm::vec4 position;
        alignas(16) glm::vec4 color;
    };

    struct GlobalUBO
    {
        alignas(16) glm::mat4 viewProj;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::vec4 cameraPos;

        alignas(16) PointLight lights[MAX_LIGHTS];
        int lightCount = 0;

        float iblIntensity = 1.0f;
        float _padding[2];
    };

    struct PushConstants
    {
        glm::mat4 model;
    };

    struct RenderObject
    {
        glm::mat4 transform;
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<MaterialPBR> material;
        uint32_t primitiveIndex;
    };

    class RendererPBR
    {
    public:
        static constexpr uint32_t k_MaxMaterials = 100;

    public:
        explicit RendererPBR(const Window &window, const RenderSettings &settings = {});
        ~RendererPBR();

        RendererPBR(const RendererPBR &) = delete;
        RendererPBR &operator=(const RendererPBR &) = delete;

        // -------------------------------------------------------
        // Render Commands
        // -------------------------------------------------------

        void BeginFrame(const Camera &camera);

        void EndFrame();

        void Submit(const RenderObject &object);

        // -------------------------------------------------------
        // required for use
        // -------------------------------------------------------

        // Must be called in the destructor of the RendererPBR owner class
        void WaitIdle() const;

        // Must be called on framebuffer reisze
        void HandleResize(uint32_t width, uint32_t height);

        // -------------------------------------------------------
        // Customization
        // -------------------------------------------------------

        void AddLight(const glm::vec3 &position, const glm::vec3 &color, float intensity = 1.0f);
        void ClearLights();

        void SetSkyboxEnabled(bool enabled) { m_SkyboxEnabled = enabled; }

        void SetSkybox(const std::array<std::string, 6> &faces);

        void SetSkybox(const std::string &directory);

        // -------------------------------------------------------
        // Helpers
        // -------------------------------------------------------

        void UploadMesh(Mesh &mesh, bool freeCPU = true) const;

        void UploadMaterial(MaterialPBR &material) const;

        void UploadTexture(Texture &texture, bool freeCPU = true) const;

        // -------------------------------------------------------
        // Getters
        // -------------------------------------------------------

        const VulkanLogicalDevice &GetLogicalDevice() const { return *m_LogicalDevice; }
        const VulkanAllocator &GetAllocator() const { return *m_Allocator; }
        const VulkanImmediateSubmit &GetGraphicsImmediateSubmit() { return *m_GraphicsImmediateSubmit; }
        const VulkanImmediateSubmit &GetTransferImmediateSubmit() { return *m_TransferImmediateSubmit; }
        const VulkanDescriptorPool &GetDescriptorPool() const { return *m_DescriptorPool; }
        bool IsSkyboxEnabled() const { return m_SkyboxEnabled; }

    private:
        void ResolveSettings(const RenderSettings &requested);

        void RecreateSwapchain();

        void RebindSkyboxDescriptor();

        void DrawSkybox(VkCommandBuffer cmd, uint32_t frameIndex);

    private:
        // Vulkan Core
        std::unique_ptr<VulkanInstance> m_Instance;
        std::unique_ptr<VulkanSurface> m_Surface;
        std::unique_ptr<VulkanPhysicalDevice> m_PhysicalDevice;
        std::unique_ptr<VulkanLogicalDevice> m_LogicalDevice;
        std::unique_ptr<VulkanAllocator> m_Allocator;

        std::unique_ptr<VulkanSwapchain> m_Swapchain;
        std::unique_ptr<VulkanDepthBuffer> m_DepthBuffer;
        std::unique_ptr<VulkanRenderPass> m_RenderPass;
        std::unique_ptr<VulkanFramebuffers> m_Framebuffers;

        std::unique_ptr<VulkanCommandPool> m_GraphicsCommandPool;
        std::unique_ptr<VulkanImmediateSubmit> m_GraphicsImmediateSubmit;

        std::unique_ptr<VulkanCommandPool> m_TransferCommandPool;
        std::unique_ptr<VulkanImmediateSubmit> m_TransferImmediateSubmit;

        // Default resources
        std::shared_ptr<Texture> m_DefaultWhiteTexture;
        std::shared_ptr<Texture> m_DefaultNormalMap;

        // Descriptors
        std::unique_ptr<VulkanDescriptorPool> m_DescriptorPool;

        // Global
        std::unique_ptr<VulkanDescriptorSetLayout> m_GlobalSetLayout;
        std::unique_ptr<VulkanDescriptorSetLayout> m_MaterialSetLayout;

        std::vector<VkDescriptorSet> m_GlobalDescriptorSets;
        std::vector<std::unique_ptr<VulkanBuffer>> m_GlobalUBOs;

        GlobalUBO m_GlobalData{};
        std::shared_ptr<Texture> m_EnvironmentMap;

        std::unique_ptr<VulkanPipelineLayout> m_PipelineLayout;
        std::unique_ptr<VulkanGraphicsPipeline> m_Pipeline;

        // Skybox
        bool m_SkyboxEnabled = false;
        std::unique_ptr<VulkanDescriptorSetLayout> m_SkyboxSetLayout;

        VkDescriptorSet m_SkyboxDescriptorSet;

        std::unique_ptr<VulkanPipelineLayout> m_SkyboxPipelineLayout;
        std::unique_ptr<VulkanGraphicsPipeline> m_SkyboxPipeline;
        std::shared_ptr<Mesh> m_SkyboxMesh;

        std::unique_ptr<VulkanFrameManager> m_FrameManager;
        uint32_t m_CurrentImageIndex = 0;

        // Resize
        bool m_NeedsResize = false;
        uint32_t m_ResizeWidth = 0;
        uint32_t m_ResizeHeight = 0;

        // Resolved render settings
        struct EffectiveSettings
        {
            float anisotropy = 1.0f;
            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
            uint32_t shadowResolution = 0;
            float iblIntensity = 1.0f;
        } m_EffectiveSettings{};
    };

} // namespace ve
