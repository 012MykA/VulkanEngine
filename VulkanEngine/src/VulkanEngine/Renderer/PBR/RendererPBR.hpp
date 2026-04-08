#pragma once

#include "VulkanEngine/Vulkan/VulkanInstance.hpp"
#include "VulkanEngine/Vulkan/VulkanSurface.hpp"
#include "VulkanEngine/Vulkan/VulkanPhysicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanAllocator.hpp"
#include "VulkanEngine/Vulkan/VulkanSwapchain.hpp"
#include "VulkanEngine/Vulkan/VulkanDepthBuffer.hpp"
#include "VulkanEngine/Vulkan/VulkanRenderPass.hpp"
#include "VulkanEngine/Vulkan/VulkanFramebuffers.hpp"
#include "VulkanEngine/Vulkan/VulkanCommandPool.hpp"
#include "VulkanEngine/Vulkan/VulkanImmediateSubmit.hpp"
#include "VulkanEngine/Vulkan/VulkanDescriptor.hpp"
#include "VulkanEngine/Vulkan/VulkanPipeline.hpp"
#include "VulkanEngine/Vulkan/VulkanFrameManager.hpp"

#include "VulkanEngine/Renderer/Mesh.hpp"
#include "VulkanEngine/Renderer/PBR/MaterialPBR.hpp"

#include <glm/glm.hpp>

#include <memory>

namespace ve
{
    class Window;
    class Camera;

    struct alignas(16) GlobalUBO
    {
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 proj = glm::mat4(1.0f);
    };

    struct PushConstants
    {
        glm::mat4 model = glm::mat4(1.0f);
    };

    struct RenderObject
    {
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<MaterialPBR> material;
        glm::mat4 transform = glm::mat4(1.0f);
    };

    class RendererPBR
    {
    public:
        static constexpr uint32_t k_MaxMaterials = 3;

    public:
        explicit RendererPBR(const Window &window);
        ~RendererPBR();

        RendererPBR(const RendererPBR &) = delete;
        RendererPBR &operator=(const RendererPBR &) = delete;

    public:
        void BeginFrame(const Camera &camera);
        void EndFrame();

        void Submit(const RenderObject &object);

    public:
        void WaitIdle() const;
        void HandleResize(uint32_t width, uint32_t height);

    public:
        void UploadMesh(Mesh &mesh) const;

        void BuildMaterial(MaterialPBR &material) const;

        std::shared_ptr<Texture> LoadTexture(const std::string &path, const TextureDesc &desc);

    private:
        void RecreateSwapchain();

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
        std::unique_ptr<VulkanDescriptorSetLayout> m_GlobalSetLayout;
        std::unique_ptr<VulkanDescriptorSetLayout> m_MaterialSetLayout;
        std::unique_ptr<VulkanDescriptorPool> m_DescriptorPool;

        GlobalUBO m_GlobalData{};
        std::vector<std::unique_ptr<VulkanBuffer>> m_GlobalUBOs;
        std::vector<VkDescriptorSet> m_GlobalDescriptorSets;

        std::unique_ptr<VulkanPipelineLayout> m_PipelineLayout;
        std::unique_ptr<VulkanGraphicsPipeline> m_Pipeline;

        std::unique_ptr<VulkanFrameManager> m_FrameManager;
        uint32_t m_CurrentImageIndex = 0;

        // Resize
        bool m_NeedsResize = false;
        uint32_t m_ResizeWidth = 0;
        uint32_t m_ResizeHeight = 0;
    };

} // namespace ve
