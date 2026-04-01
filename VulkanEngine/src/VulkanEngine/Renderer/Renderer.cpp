#include "Renderer.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "VulkanEngine/Core/Timer.hpp"
#include "VulkanEngine/Vulkan/Debug/VulkanValidation.hpp"

namespace ve
{
    Renderer::Renderer(const Window &window)
    {
        Init(window);
    }

    Renderer::~Renderer()
    {
        if (m_LogicalDevice)
            m_LogicalDevice->WaitIdle();
    }

    void Renderer::BeginFrame(const Camera &camera)
    {
        auto &frame = m_FrameManager->GetCurrentFrame();
        const uint32_t frameIndex = m_FrameManager->GetCurrentFrameIndex();

        frame.syncObjects->WaitForFence();

        VkResult result = vkAcquireNextImageKHR(
            m_LogicalDevice->GetVkHandle(),
            m_Swapchain->GetVkHandle(),
            UINT64_MAX,
            frame.syncObjects->GetImageAvailableSemaphore(),
            VK_NULL_HANDLE,
            &m_CurrentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
            RecreateSwapchain();

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            CHECK_VK_RESULT(result);
        }

        frame.syncObjects->ResetFence();

        CameraUBO cameraData{
            .view = camera.GetViewMatrix(),
            .proj = camera.GetProjectionMatrix(),
        };
        m_CameraUBOs[frameIndex]->Upload(&cameraData, sizeof(CameraUBO));

        VkCommandBuffer cmd = frame.commandBuffer;
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        result = vkBeginCommandBuffer(cmd, &beginInfo);
        CHECK_VK_RESULT(result);

        // RenderPass
        VkClearValue clearColor = {
            .color = {{0.1f, 0.1f, 0.1f, 1.0f}},
        };

        VkRenderPassBeginInfo rpBegin{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = m_RenderPass->GetVkHandle(),
            .framebuffer = m_Framebuffers->GetFramebuffer(m_CurrentImageIndex),
            .renderArea{
                .offset = {0, 0},
                .extent = m_Swapchain->GetExtent(),
            },
            .clearValueCount = 1,
            .pClearValues = &clearColor,
        };

        vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

        // Dynamic viewport and scissor
        VkExtent2D extent = m_Swapchain->GetExtent();

        VkViewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(extent.width),
            .height = static_cast<float>(extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{
            .offset = {0, 0},
            .extent = extent,
        };
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void Renderer::EndFrame()
    {
        auto &frame = m_FrameManager->GetCurrentFrame();
        VkCommandBuffer cmd = frame.commandBuffer;

        // End render pass and command buffer
        vkCmdEndRenderPass(cmd);
        VkResult result = vkEndCommandBuffer(cmd);
        CHECK_VK_RESULT(result);

        // Submit
        VkSemaphore waitSem = frame.syncObjects->GetImageAvailableSemaphore();
        VkSemaphore signalSem = frame.syncObjects->GetRenderFinishedSemaphore();
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &waitSem,
            .pWaitDstStageMask = &waitStage,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &signalSem,
        };

        result = vkQueueSubmit(m_LogicalDevice->GetGraphicsQueue(), 1, &submitInfo, frame.syncObjects->GetInFlightFence());
        CHECK_VK_RESULT(result);

        // Present
        VkSwapchainKHR swapchains[] = {m_Swapchain->GetVkHandle()};

        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &signalSem,
            .swapchainCount = 1,
            .pSwapchains = swapchains,
            .pImageIndices = &m_CurrentImageIndex,
        };

        result = vkQueuePresentKHR(m_LogicalDevice->GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_NeedsResize)
        {
            m_NeedsResize = false;
            RecreateSwapchain();
        }
        else
        {
            CHECK_VK_RESULT(result);
        }

        m_FrameManager->AdvanceFrame();
    }

    void Renderer::Submit()
    {
        const uint32_t frameIndex = m_FrameManager->GetCurrentFrameIndex();
        auto &frame = m_FrameManager->GetCurrentFrame();
        VkCommandBuffer cmd = frame.commandBuffer;

        m_Pipeline->Bind(cmd);

        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout->GetVkHandle(),
            0,
            1, &m_GlobalDescriptorSets[frameIndex],
            0, nullptr);

        PushConstants pc{
            .model = glm::mat4(1.0f),
        };
        vkCmdPushConstants(
            cmd,
            m_PipelineLayout->GetVkHandle(),
            VK_SHADER_STAGE_VERTEX_BIT,
            0, sizeof(PushConstants), &pc);

        m_TriangleMesh->Bind(cmd);
        vkCmdDrawIndexed(cmd, m_TriangleMesh->GetIndexCount(), 1, 0, 0, 0);
    }

    void Renderer::HandleResize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        m_NeedsResize = true;
        m_ResizeWidth = width;
        m_ResizeHeight = height;
    }

    void Renderer::Init(const Window &window)
    {
        // Instance
        m_Instance = std::make_unique<VulkanInstance>(InstanceDesc{
            .requiredExtensions = window.GetRequiredVulkanExtensions(),
#ifdef VE_DEBUG
            .enableValidation = true,
            .debugMessenger{
                .enableDebugMessenger = true,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            },
#endif
        });

        m_Surface = std::make_unique<VulkanSurface>(*m_Instance, window);
        m_PhysicalDevice = std::make_unique<VulkanPhysicalDevice>(*m_Instance, *m_Surface);
        m_LogicalDevice = std::make_unique<VulkanLogicalDevice>(*m_PhysicalDevice, LogicalDeviceDesc{});
        m_Allocator = std::make_unique<VulkanAllocator>(*m_Instance, *m_PhysicalDevice, *m_LogicalDevice);

        m_Swapchain = std::make_unique<VulkanSwapchain>(
            *m_PhysicalDevice,
            *m_LogicalDevice,
            *m_Surface,
            SwapchainDesc{
                .width = window.GetWidth(),
                .height = window.GetHeight(),
            });

        m_RenderPass = std::make_unique<VulkanRenderPass>(*m_LogicalDevice, m_Swapchain->GetFormat());
        m_Framebuffers = std::make_unique<VulkanFramebuffers>(*m_LogicalDevice, *m_Swapchain, *m_RenderPass);
        m_GraphicsCommandPool = std::make_unique<VulkanCommandPool>(
            *m_LogicalDevice, *m_PhysicalDevice,
            CommandPoolDesc{.type = CommandPoolType::Graphics, .resetBuffer = true});

        m_TransferCommandPool = std::make_unique<VulkanCommandPool>(
            *m_LogicalDevice, *m_PhysicalDevice,
            CommandPoolDesc{.type = CommandPoolType::Transfer, .transient = true});

        m_FrameManager = std::make_unique<VulkanFrameManager>(*m_LogicalDevice, *m_GraphicsCommandPool);
        m_ImmediateSubmit = std::make_unique<VulkanImmediateSubmit>(*m_LogicalDevice, *m_TransferCommandPool);

        // Descriptors
        m_GlobalSetLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayout::Builder(*m_LogicalDevice)
                .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .Build());

        m_DescriptorPool = std::make_unique<VulkanDescriptorPool>(
            *m_LogicalDevice,
            DescriptorPoolDesc{
                .maxSets = VulkanFrameManager::k_MaxFramesInFlight,
                .poolSizes = {
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VulkanFrameManager::k_MaxFramesInFlight},
                },
            });

        m_CameraUBOs.resize(VulkanFrameManager::k_MaxFramesInFlight);
        m_GlobalDescriptorSets.resize(VulkanFrameManager::k_MaxFramesInFlight);

        for (uint32_t i = 0; i < VulkanFrameManager::k_MaxFramesInFlight; i++)
        {
            m_CameraUBOs[i] = std::make_unique<VulkanBuffer>(*m_Allocator, MakeUniformBufferDesc(sizeof(CameraUBO)));

            m_GlobalDescriptorSets[i] = m_DescriptorPool->Allocate(m_GlobalSetLayout->GetVkHandle());

            VkDescriptorBufferInfo bufferInfo{
                .buffer = m_CameraUBOs[i]->GetVkHandle(),
                .offset = 0,
                .range = sizeof(CameraUBO),
            };

            VulkanDescriptorWriter(m_LogicalDevice->GetVkHandle())
                .WriteBuffer(0,
                             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                             m_GlobalDescriptorSets[i],
                             bufferInfo)
                .Flush();
        }

        // Pipeline
        m_PipelineLayout = std::make_unique<VulkanPipelineLayout>(
            VulkanPipelineLayout::Builder(*m_LogicalDevice)
                .AddDescriptorSetLayout(m_GlobalSetLayout->GetVkHandle())
                .AddPushConstantRange<PushConstants>(VK_SHADER_STAGE_VERTEX_BIT)
                .Build());

        VulkanShader vertexShader(*m_LogicalDevice, "../VulkanEngine/assets/shaders/triangle.vert.spv");
        VulkanShader fragmentShader(*m_LogicalDevice, "../VulkanEngine/assets/shaders/triangle.frag.spv");

        GraphicsPipelineDesc pipelineDesc{
            .vertexShader = vertexShader.GetVkHandle(),
            .fragmentShader = fragmentShader.GetVkHandle(),

            .vertexInput{
                .bindings = {
                    VkVertexInputBindingDescription{.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
                },
                .attributes = {
                    {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, position)},
                    {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, normal)},
                    {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex, tangent)},
                    {.location = 3, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, uv)},
                }},

            // .cullMode = VK_CULL_MODE_NONE,

            .depthTest = false,
            .depthWrite = false,

            .colorBlendAttachment = MakeOpaqueBlend(),

            .renderPass = m_RenderPass->GetVkHandle(),
            .layout = m_PipelineLayout->GetVkHandle(),
        };

        m_Pipeline = std::make_unique<VulkanGraphicsPipeline>(*m_LogicalDevice, pipelineDesc);

        // TODO: remove
        const std::vector<Vertex> vertices = {
            // position                // normal         // tangent               // uv
            {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        };

        const std::vector<uint32_t> indices = {0, 2, 1, 0, 3, 2};

        m_TriangleMesh = std::make_unique<Mesh>();
        m_TriangleMesh->SetName("Quad");
        m_TriangleMesh->SetVertices(vertices);
        m_TriangleMesh->SetIndices(indices);

        m_TriangleMesh->UploadToGPU(*m_Allocator, *m_ImmediateSubmit);
        // ---
    }

    void Renderer::RecreateSwapchain()
    {
        m_LogicalDevice->WaitIdle();

        Timer timer;

        m_Swapchain->Recreate(m_ResizeWidth, m_ResizeHeight);
        m_Framebuffers->Recreate(*m_Swapchain, *m_RenderPass);

        VE_CORE_TRACE("Swapchain recreated: {}x{} ({} ms)", m_ResizeWidth, m_ResizeHeight, timer.ElapsedMilliseconds());
    }

} // namespace ve
