#include "RendererPBR.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "VulkanEngine/Renderer/Camera/Camera.hpp"
#include "Backends/Vulkan/VulkanShader.hpp"
#include "VulkanEngine/Assets/MeshLoader.hpp"
#include "VulkanEngine/Assets/TextureLoader.hpp"

#include "VulkanEngine/Core/Timer.hpp"
#include "Backends/Vulkan/Debug/VulkanValidation.hpp"

#include <cassert>
#include <vector>
#include <fstream>

#include "VulkanEngine/Core/Log.hpp"

#define PBR_VERTEX_SHADER_PATH "../VulkanEngine/assets/shaders/bin/pbr_basic.vert.spv"
#define PBR_FRAGMENT_SHADER_PATH "../VulkanEngine/assets/shaders/bin/pbr_basic.frag.spv"

#define SKYBOX_VERTEX_SHADER_PATH "../VulkanEngine/assets/shaders/bin/skybox.vert.spv"
#define SKYBOX_FRAGMENT_SHADER_PATH "../VulkanEngine/assets/shaders/bin/skybox.frag.spv"

#define PIPELINE_CACHE_PATH "../VulkanEngine/assets/cache/pipeline_cache.bin"

namespace ve
{
    RendererPBR::RendererPBR(const Window &window, const RenderSettings &settings)
    {
        m_Instance = std::make_unique<VulkanInstance>(
            InstanceDesc{
                .requiredExtensions = window.GetRequiredVulkanExtensions(),
#ifdef VE_DEBUG
                .enableValidation = true,
                .debugMessenger{
                    .enableDebugMessenger = true,
                    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                },
#else
                .enableValidation = false,
                .debugMessenger{
                    .enableDebugMessenger = false,
                },
#endif
            });

        m_Surface = std::make_unique<VulkanSurface>(*m_Instance, window);
        m_PhysicalDevice = std::make_unique<VulkanPhysicalDevice>(*m_Instance, *m_Surface);

        ResolveSettings(settings);

        const auto &caps = m_PhysicalDevice->GetCapabilities();
        m_LogicalDevice = std::make_unique<VulkanLogicalDevice>(
            *m_PhysicalDevice,
            LogicalDeviceDesc{
                .enabledFeatures{
                    .geometryShader = caps.geometryShaderSupported ? VK_TRUE : VK_FALSE,
                    .samplerAnisotropy = (m_EffectiveSettings.anisotropy > 1.0f) ? VK_TRUE : VK_FALSE,
                },
            });

        m_Allocator = std::make_unique<VulkanAllocator>(*m_Instance, *m_PhysicalDevice, *m_LogicalDevice);

        m_Swapchain = std::make_unique<VulkanSwapchain>(
            *m_PhysicalDevice,
            *m_LogicalDevice,
            *m_Surface,
            SwapchainDesc{
                .width = window.GetWidth(),
                .height = window.GetHeight(),
            });

        const bool useMsaa = (m_EffectiveSettings.msaaSamples > VK_SAMPLE_COUNT_1_BIT);
        if (useMsaa)
        {
            m_MsaaColorBuffer = std::make_unique<VulkanImage>(
                *m_Allocator, *m_LogicalDevice,
                ImageDesc{
                    .width = m_Swapchain->GetExtent().width,
                    .height = m_Swapchain->GetExtent().height,
                    .format = m_Swapchain->GetFormat(),
                    .type = ImageType::ColorAttachment,
                    .samples = m_EffectiveSettings.msaaSamples,
                });
        }

        m_DepthBuffer = std::make_unique<VulkanDepthBuffer>(
            *m_LogicalDevice,
            *m_PhysicalDevice,
            *m_Allocator,
            m_Swapchain->GetExtent().width,
            m_Swapchain->GetExtent().height,
            m_EffectiveSettings.msaaSamples);

        m_RenderPass = std::make_unique<VulkanRenderPass>(
            *m_LogicalDevice,
            m_Swapchain->GetFormat(),
            m_DepthBuffer->GetFormat(),
            m_EffectiveSettings.msaaSamples);

        VkImageView msaaView = m_MsaaColorBuffer ? m_MsaaColorBuffer->GetView() : VK_NULL_HANDLE;
        m_Framebuffers = std::make_unique<VulkanFramebuffers>(
            *m_LogicalDevice,
            *m_Swapchain,
            *m_RenderPass,
            *m_DepthBuffer,
            msaaView);

        m_GraphicsCommandPool = std::make_unique<VulkanCommandPool>(
            *m_LogicalDevice, *m_PhysicalDevice,
            CommandPoolDesc{.type = CommandPoolType::Graphics, .resetBuffer = true});

        m_GraphicsImmediateSubmit = std::make_unique<VulkanImmediateSubmit>(
            *m_LogicalDevice,
            *m_GraphicsCommandPool,
            m_LogicalDevice->GetGraphicsQueue());

        m_TransferCommandPool = std::make_unique<VulkanCommandPool>(
            *m_LogicalDevice, *m_PhysicalDevice,
            CommandPoolDesc{.type = CommandPoolType::Transfer, .transient = true});

        m_TransferImmediateSubmit = std::make_unique<VulkanImmediateSubmit>(
            *m_LogicalDevice,
            *m_TransferCommandPool,
            m_LogicalDevice->GetTransferQueue());

        // Default resources
        m_DefaultWhiteTexture = TextureLoader().CreateSolid(255, 255, 255, 255);
        UploadTexture(*m_DefaultWhiteTexture);

        m_DefaultNormalMap = TextureLoader().CreateSolid(128, 128, 255, 255);
        UploadTexture(*m_DefaultNormalMap);

        // Descriptors
        constexpr uint32_t framesInFlight = VulkanFrameManager::k_MaxFramesInFlight;
        constexpr uint32_t maxMats = k_MaxMaterials;

        // Sets: global * frames + skybox + materials
        constexpr uint32_t maxSets = framesInFlight + 1 + maxMats;

        // UBOs: global UBO * frames + material UBO * mats
        constexpr uint32_t uboCount = framesInFlight + maxMats;

        // Samplers: IBL(3) * frames + skybox(1) + material textures(6) * mats
        constexpr uint32_t samplerCount = (3 * framesInFlight) + 1 + (6 * maxMats);

        m_DescriptorPool = std::make_unique<VulkanDescriptorPool>(
            *m_LogicalDevice,
            DescriptorPoolDesc{
                .maxSets = maxSets,
                .poolSizes = {
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uboCount},
                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, samplerCount},
                },
            });

        // Global
        m_GlobalSetLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayout::Builder(*m_LogicalDevice)
                .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                .Build());

        m_MaterialSetLayout = std::make_unique<VulkanDescriptorSetLayout>(MaterialPBR::CreateLayout(*m_LogicalDevice));

        m_GlobalUBOs.resize(VulkanFrameManager::k_MaxFramesInFlight);
        m_GlobalDescriptorSets.resize(VulkanFrameManager::k_MaxFramesInFlight);

        for (uint32_t i = 0; i < VulkanFrameManager::k_MaxFramesInFlight; i++)
        {
            m_GlobalUBOs[i] = std::make_unique<VulkanBuffer>(*m_Allocator, MakeUniformBufferDesc(sizeof(GlobalUBO)));

            m_GlobalDescriptorSets[i] = m_DescriptorPool->Allocate(m_GlobalSetLayout->GetVkHandle());

            VkDescriptorBufferInfo bufferInfo{
                .buffer = m_GlobalUBOs[i]->GetVkHandle(),
                .offset = 0,
                .range = sizeof(GlobalUBO),
            };

            VulkanDescriptorWriter(m_LogicalDevice->GetVkHandle())
                .WriteBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_GlobalDescriptorSets[i], bufferInfo)
                .Flush();
        }

        m_PipelineLayout = std::make_unique<VulkanPipelineLayout>(
            VulkanPipelineLayout::Builder(*m_LogicalDevice)
                .AddDescriptorSetLayout(m_GlobalSetLayout->GetVkHandle())
                .AddDescriptorSetLayout(m_MaterialSetLayout->GetVkHandle())
                .AddPushConstantRange<PushConstants>(VK_SHADER_STAGE_VERTEX_BIT)
                .Build());

        VulkanShader pbrVertexShader(*m_LogicalDevice, PBR_VERTEX_SHADER_PATH);
        VulkanShader pbrFragmentShader(*m_LogicalDevice, PBR_FRAGMENT_SHADER_PATH);

        ve::Timer startupTimer;

        // Pipeline cache
        std::vector<char> pipelineCacheData = VulkanPipelineCache::LoadCacheBinary(PIPELINE_CACHE_PATH);
        if (!pipelineCacheData.empty())
        {
            VkPhysicalDeviceProperties deviceProps = m_PhysicalDevice->GetProperties();
            if (!VulkanPipelineCache::IsCacheDataValid(pipelineCacheData, deviceProps))
            {
                VE_CORE_WARN("Pipeline cache file is incompatible with the current GPU/driver, ignoring it");
                pipelineCacheData.clear();
            }
        }

        VulkanPipelineCache pipelineCache(*m_LogicalDevice, pipelineCacheData);

        m_Pipeline = std::make_unique<VulkanGraphicsPipeline>(
            *m_LogicalDevice,
            GraphicsPipelineDesc{
                .vertexShader = pbrVertexShader.GetVkHandle(),
                .fragmentShader = pbrFragmentShader.GetVkHandle(),
                .vertexInput = GetPBRVertexInputDesc(),
                .samples = m_EffectiveSettings.msaaSamples,
                .depthTest = true,
                .depthWrite = true,
                .colorBlendAttachment = MakeOpaqueBlend(),
                .renderPass = m_RenderPass->GetVkHandle(),
                .layout = m_PipelineLayout->GetVkHandle(),
            },
            &pipelineCache);

        VE_CORE_INFO("Pipeline creation completed in {} ms", startupTimer.ElapsedMilliseconds());

        // Skybox
        m_SkyboxSetLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayout::Builder(*m_LogicalDevice)
                .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .Build());

        m_SkyboxDescriptorSet = m_DescriptorPool->Allocate(m_SkyboxSetLayout->GetVkHandle());

        m_SkyboxPipelineLayout = std::make_unique<VulkanPipelineLayout>(
            VulkanPipelineLayout::Builder(*m_LogicalDevice)
                .AddDescriptorSetLayout(m_GlobalSetLayout->GetVkHandle())
                .AddDescriptorSetLayout(m_SkyboxSetLayout->GetVkHandle())
                .Build());

        VulkanShader skyboxVertexShader(*m_LogicalDevice, SKYBOX_VERTEX_SHADER_PATH);
        VulkanShader skyboxFragmentShader(*m_LogicalDevice, SKYBOX_FRAGMENT_SHADER_PATH);

        m_SkyboxPipeline = std::make_unique<VulkanGraphicsPipeline>(
            *m_LogicalDevice,
            GraphicsPipelineDesc{
                .vertexShader = skyboxVertexShader.GetVkHandle(),
                .fragmentShader = skyboxFragmentShader.GetVkHandle(),
                .vertexInput{
                    .bindings = {
                        VkVertexInputBindingDescription{
                            .binding = 0,
                            .stride = sizeof(Vertex),
                            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
                    },
                    .attributes = {
                        {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, position)},
                    },
                },
                .cullMode = VK_CULL_MODE_NONE,
                .samples = m_EffectiveSettings.msaaSamples,
                .depthTest = true,
                .depthWrite = false,
                .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
                .colorBlendAttachment = MakeOpaqueBlend(),
                .renderPass = m_RenderPass->GetVkHandle(),
                .layout = m_SkyboxPipelineLayout->GetVkHandle(),
            },
            &pipelineCache);

        pipelineCache.SaveCacheToFile(PIPELINE_CACHE_PATH);

        m_SkyboxMesh = MeshLoader().LoadGLTF("../VulkanEngine/assets/models/Cube.glb");
        UploadMesh(*m_SkyboxMesh);

        m_FrameManager = std::make_unique<VulkanFrameManager>(*m_LogicalDevice, *m_GraphicsCommandPool);
    }

    RendererPBR::~RendererPBR()
    {
        WaitIdle();
    }

    void RendererPBR::BeginFrame(const Camera &camera)
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

        // Updating global UBO
        m_GlobalData.viewProj = camera.GetViewProjection();
        m_GlobalData.view = camera.GetView();
        m_GlobalData.proj = camera.GetProjection();
        m_GlobalData.cameraPos = glm::vec4(camera.GetPosition(), 0.0f);
        m_GlobalUBOs[frameIndex]->Upload(&m_GlobalData, sizeof(GlobalUBO));
        // ---

        VkCommandBuffer cmd = frame.commandBuffer;
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        result = vkBeginCommandBuffer(cmd, &beginInfo);
        CHECK_VK_RESULT(result);

        // RenderPass
        const bool useMsaa = (m_EffectiveSettings.msaaSamples > VK_SAMPLE_COUNT_1_BIT);
        std::vector<VkClearValue> clearValues(useMsaa ? 3 : 2);
        clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
        // clearValues[3] = ...

        VkRenderPassBeginInfo rpBegin{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = m_RenderPass->GetVkHandle(),
            .framebuffer = m_Framebuffers->GetFramebuffer(m_CurrentImageIndex),
            .renderArea{
                .offset = {0, 0},
                .extent = m_Swapchain->GetExtent(),
            },
            .clearValueCount = static_cast<uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data(),
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

        if (m_SkyboxEnabled)
            DrawSkybox(cmd, frameIndex);
    }

    void RendererPBR::EndFrame()
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

#ifdef VE_USE_RENDERER_API_V1

    void RendererPBR::Submit(const RenderObject &object)
    {
        const uint32_t frameIndex = m_FrameManager->GetCurrentFrameIndex();
        auto &frame = m_FrameManager->GetCurrentFrame();
        VkCommandBuffer cmd = frame.commandBuffer;

        m_Pipeline->Bind(cmd);

        VkDescriptorSet globalSet = m_GlobalDescriptorSets[frameIndex];
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout->GetVkHandle(),
            0, 1, &globalSet,
            0, nullptr);

        PushConstants pc{.model = object.transform};
        vkCmdPushConstants(
            cmd,
            m_PipelineLayout->GetVkHandle(),
            VK_SHADER_STAGE_VERTEX_BIT,
            0, sizeof(PushConstants), &pc);

        assert(object.mesh && "object.mesh should be a valid pointer");
        object.mesh->Bind(cmd);

        assert(object.material && "object.material should be a valid pointer");
        object.material->Bind(cmd, m_PipelineLayout->GetVkHandle(), 1);

        const auto &primitives = object.mesh->GetPrimitives();
        assert(object.primitiveIndex < primitives.size());
        const Primitive &primitive = primitives[object.primitiveIndex];

        vkCmdDrawIndexed(
            cmd,
            primitive.indexCount,
            1,
            primitive.firstIndex,
            0, 0);
    }

#endif // VE_USE_RENDERER_API_V1

    void RendererPBR::BindPipeline(VkCommandBuffer cmd)
    {
        m_Pipeline->Bind(cmd);
    }

    void RendererPBR::BindGlobalDescriptorSet(VkCommandBuffer cmd, uint32_t frameIndex)
    {
        VkDescriptorSet globalSet = m_GlobalDescriptorSets[frameIndex];
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout->GetVkHandle(),
            0, 1, &globalSet,
            0, nullptr);
    }

    void RendererPBR::PushData(VkCommandBuffer cmd, const PushConstants &push)
    {
        vkCmdPushConstants(
            cmd,
            m_PipelineLayout->GetVkHandle(),
            VK_SHADER_STAGE_VERTEX_BIT,
            0, sizeof(PushConstants), &push);
    }

    void RendererPBR::BindMaterial(VkCommandBuffer cmd, const std::shared_ptr<MaterialPBR> &material)
    {
        assert(material && "object.material should be a valid pointer");

        material->Bind(cmd, m_PipelineLayout->GetVkHandle(), 1);
    }

    void RendererPBR::DrawIndexed(VkCommandBuffer cmd, const Primitive &primitive)
    {
        vkCmdDrawIndexed(
            cmd,
            primitive.indexCount,
            1,
            primitive.firstIndex,
            0, 0);
    }

    void RendererPBR::WaitIdle() const
    {
        if (m_LogicalDevice)
            m_LogicalDevice->WaitIdle();
    }

    void RendererPBR::HandleResize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        m_NeedsResize = true;
        m_ResizeWidth = width;
        m_ResizeHeight = height;
    }

    void RendererPBR::AddLight(const gltf::Light &light, const glm::mat4 &worldTransform)
    {
        if (m_GlobalData.lightCount >= MAX_LIGHTS)
        {
            VE_CORE_WARN("Unable to AddLight. Limit reached ({})", MAX_LIGHTS);
            return;
        }

        ShaderLight &sl = m_GlobalData.lights[m_GlobalData.lightCount];

        sl.position = glm::vec4(glm::vec3(worldTransform[3]), static_cast<float>(light.type));
        sl.color = glm::vec4(light.color, light.intensity);

        glm::vec3 dir = glm::normalize(glm::vec3(worldTransform * glm::vec4(0, 0, -1, 0)));
        sl.direction = glm::vec4(dir, light.range);

        sl.coneAngles = glm::vec4(
            glm::cos(light.innerConeAngle),
            glm::sin(light.outerConeAngle),
            0.0f, 0.0f);

        m_GlobalData.lightCount++;
    }

    void RendererPBR::ClearLights()
    {
        m_GlobalData.lightCount = 0;
    }

    void RendererPBR::SetSkybox(const std::array<std::string, 6> &faces)
    {
        WaitIdle();

        m_EnvironmentMap = TextureLoader().LoadCubeMap(faces);
        UploadTexture(*m_EnvironmentMap);

        RebindSkyboxDescriptor();
    }

    void RendererPBR::SetSkybox(const std::string &directory)
    {
        std::array<std::string, 6> faces = {
            directory + "px.hdr", // +X
            directory + "nx.hdr", // -X
            directory + "py.hdr", // +Y
            directory + "ny.hdr", // -Y
            directory + "pz.hdr", // +Z
            directory + "nz.hdr", // -Z
        };
        SetSkybox(faces);
    }

    void RendererPBR::UploadMesh(Mesh &mesh, bool freeCPU) const
    {
        mesh.Upload(*m_Allocator, *m_TransferImmediateSubmit);

        if (freeCPU)
            mesh.FreeCPUData();
    }

    void RendererPBR::UploadMaterial(MaterialPBR &material) const
    {
        material.Upload(
            *m_Allocator,
            *m_DescriptorPool,
            *m_MaterialSetLayout,
            *m_LogicalDevice,
            *m_DefaultWhiteTexture,
            *m_DefaultNormalMap);
    }

    void RendererPBR::UploadTexture(Texture &texture, bool freeCPU) const
    {
        SamplerDesc samplerDesc{};
        samplerDesc.maxAnisotropy = m_EffectiveSettings.anisotropy;

        texture.Upload(
            *m_Allocator,
            *m_LogicalDevice,
            *m_GraphicsImmediateSubmit,
            samplerDesc);

        if (freeCPU)
            texture.FreeCPUData();
    }

    void RendererPBR::ResolveSettings(const RenderSettings &requested)
    {
        const DeviceCapabilities &caps = m_PhysicalDevice->GetCapabilities();

        // Anisotropy
        float requestedAnisotropy = static_cast<float>(requested.anisotropy);
        m_EffectiveSettings.anisotropy = caps.ClampAnisotropy(requestedAnisotropy);

        // MSAA
        VkSampleCountFlagBits requestedSamples = static_cast<VkSampleCountFlagBits>(requested.msaaSamples);
        m_EffectiveSettings.msaaSamples = caps.ClampMSAASamples(requestedSamples);

        // Other settings
        m_EffectiveSettings.shadowResolution = static_cast<uint32_t>(requested.shadowQuality);
        m_EffectiveSettings.iblIntensity = requested.iblIntensity;
    }

    void RendererPBR::RecreateSwapchain()
    {
        WaitIdle();

        Timer timer;

        m_Swapchain->Recreate(m_ResizeWidth, m_ResizeHeight);

        const bool useMsaa = (m_EffectiveSettings.msaaSamples > VK_SAMPLE_COUNT_1_BIT);
        if (useMsaa)
        {
            m_MsaaColorBuffer = std::make_unique<VulkanImage>(
                *m_Allocator, *m_LogicalDevice,
                ImageDesc{
                    .width = m_ResizeWidth,
                    .height = m_ResizeHeight,
                    .format = m_Swapchain->GetFormat(),
                    .type = ImageType::ColorAttachment,
                    .samples = m_EffectiveSettings.msaaSamples,
                });
        }
        m_DepthBuffer->Recreate(*m_Allocator,
                                m_ResizeWidth,
                                m_ResizeHeight,
                                m_EffectiveSettings.msaaSamples);

        VkImageView msaaView = m_MsaaColorBuffer ? m_MsaaColorBuffer->GetView() : VK_NULL_HANDLE;
        m_Framebuffers->Recreate(*m_Swapchain,
                                 *m_RenderPass,
                                 *m_DepthBuffer,
                                 msaaView);

        VE_CORE_TRACE("Swapchain recreated: {}x{} ({} ms)", m_ResizeWidth, m_ResizeHeight, timer.ElapsedMilliseconds());
    }

    void RendererPBR::RebindSkyboxDescriptor()
    {
        VkDescriptorImageInfo envImageInfo = m_EnvironmentMap->GetDescriptorInfo();
        VulkanDescriptorWriter(m_LogicalDevice->GetVkHandle())
            .WriteImage(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_SkyboxDescriptorSet, envImageInfo)
            .Flush();
    }

    void RendererPBR::DrawSkybox(VkCommandBuffer cmd, uint32_t frameIndex)
    {
        m_SkyboxPipeline->Bind(cmd);

        VkDescriptorSet sets[] = {
            m_GlobalDescriptorSets[frameIndex],
            m_SkyboxDescriptorSet,
        };

        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_SkyboxPipelineLayout->GetVkHandle(),
            0, 2, sets,
            0, nullptr);

        m_SkyboxMesh->Bind(cmd);
        for (const auto &primitive : m_SkyboxMesh->GetPrimitives())
        {
            vkCmdDrawIndexed(cmd, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
        }
    }

} // namespace ve
