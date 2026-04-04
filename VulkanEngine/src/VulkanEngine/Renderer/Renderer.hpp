#pragma once

#include "VulkanEngine/Vulkan/VulkanInstance.hpp"
#include "VulkanEngine/Vulkan/VulkanSurface.hpp"
#include "VulkanEngine/Vulkan/VulkanPhysicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanAllocator.hpp"
#include "VulkanEngine/Vulkan/VulkanSwapchain.hpp"
#include "VulkanEngine/Vulkan/VulkanRenderPass.hpp"
#include "VulkanEngine/Vulkan/VulkanDepthBuffer.hpp"
#include "VulkanEngine/Vulkan/VulkanFramebuffers.hpp"
#include "VulkanEngine/Vulkan/VulkanCommandPool.hpp"
#include "VulkanEngine/Vulkan/VulkanFrameData.hpp"
#include "VulkanEngine/Vulkan/VulkanImmediateSubmit.hpp"
#include "VulkanEngine/Vulkan/VulkanDescriptor.hpp"
#include "VulkanEngine/Vulkan/VulkanPipeline.hpp"

#include "Camera.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

#include <glm/glm.hpp>

#include <memory>

namespace ve
{
    class Window;

    struct GlobalUBO
    {
        alignas(16) glm::mat4 view = glm::mat4(1.0f);
        alignas(16) glm::mat4 proj = glm::mat4(1.0f);
        alignas(16) glm::vec4 lightDir = glm::vec4(glm::normalize(glm::vec3(-0.3f, -0.8f, -0.5f)), 0.0f);
        alignas(16) glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // w = intensity
        alignas(16) glm::vec4 cameraPos = glm::vec4(0.0f);
    };

    struct PushConstants
    {
        glm::mat4 model = glm::mat4(1.0f);
    };

    class Renderer
    {
    public:
        explicit Renderer(const Window &window);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        void BeginFrame(const Camera &camera);
        void EndFrame();

        void Submit(const Mesh &mesh, const Material &material, const glm::mat4 &transform);

        void SetLight(const glm::vec3 &direction, const glm::vec3 &color = glm::vec3(1.0f), float intencity = 1.0f);

        void WaitIdle() const;
        void HandleResize(uint32_t width, uint32_t height);

    public: // Upload data
        void UploadMesh(Mesh &mesh) const;
        void BuildMaterial(Material &material) const;

    public: // Getters
        const VulkanAllocator &GetAllocator() const { return *m_Allocator; }
        const VulkanImmediateSubmit &GetImmediateSubmit() { return *m_ImmediateSubmit; }

    private:
        void Init(const Window &window);
        void RecreateSwapchain();

    private:
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
        std::unique_ptr<VulkanCommandPool> m_TransferCommandPool;
        std::unique_ptr<VulkanFrameManager> m_FrameManager;
        std::unique_ptr<VulkanImmediateSubmit> m_ImmediateSubmit;

        // Descriptors
        std::unique_ptr<VulkanDescriptorSetLayout> m_GlobalSetLayout;   // set = 0, binding = 0
        std::unique_ptr<VulkanDescriptorSetLayout> m_MaterialSetLayout; // set = 1, binding = 0
        std::unique_ptr<VulkanDescriptorPool> m_DescriptorPool;

        std::vector<std::unique_ptr<VulkanBuffer>> m_GlobalUBOs;
        GlobalUBO m_GlobalData{};
        std::vector<VkDescriptorSet> m_GlobalDescriptorSets;

        // Pipeline
        std::unique_ptr<VulkanPipelineLayout> m_PipelineLayout;
        std::unique_ptr<VulkanGraphicsPipeline> m_Pipeline;

        uint32_t m_CurrentImageIndex = 0;

        bool m_NeedsResize = false;
        uint32_t m_ResizeWidth = 0, m_ResizeHeight = 0;
    };

} // namespace ve
