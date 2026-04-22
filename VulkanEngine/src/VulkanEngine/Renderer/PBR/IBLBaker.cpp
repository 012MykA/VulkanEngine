// #include "IBLBaker.hpp"

// #include "Backends/Vulkan/VulkanAllocator.hpp"
// #include "Backends/Vulkan/VulkanLogicalDevice.hpp"
// #include "Backends/Vulkan/VulkanImmediateSubmit.hpp"
// #include "Backends/Vulkan/VulkanBuffer.hpp"
// #include "VulkanEngine/Core/Log.hpp"
// #include "VulkanEngine/Core/Timer.hpp"

// #include <glm/glm.hpp>
// #include <glm/gtc/constants.hpp>

// #include <vector>
// #include <array>
// #include <future>
// #include <cmath>
// #include <algorithm>

// namespace ve
// {
//     struct FaceAxes
//     {
//         glm::vec3 dir;
//         glm::vec3 right;
//         glm::vec3 up;
//     };

//     static constexpr std::array<FaceAxes, 6> k_Faces{{
//         {{1, 0, 0}, {0, 0, -1}, {0, -1, 0}},  // +X
//         {{-1, 0, 0}, {0, 0, 1}, {0, -1, 0}},  // -X
//         {{0, 1, 0}, {1, 0, 0}, {0, 0, 1}},    // +Y
//         {{0, -1, 0}, {1, 0, 0}, {0, 0, -1}},  // -Y
//         {{0, 0, 1}, {1, 0, 0}, {0, -1, 0}},   // +Z
//         {{0, 0, -1}, {-1, 0, 0}, {0, -1, 0}}, // -Z
//     }};

//     static glm::vec3 SampleCubemapCPU(
//         const float *srcData,
//         uint32_t faceSize,
//         glm::vec3 dir)
//     {
//         dir = glm::normalize(dir);

//         float ax = std::abs(dir.x), ay = std::abs(dir.y), az = std::abs(dir.z);
//         uint32_t face;
//         float sc, tc, ma;

//         if (ax >= ay && ax >= az)
//         {
//             ma = ax;
//             if (dir.x > 0)
//             {
//                 face = 0;
//                 sc = -dir.z;
//                 tc = -dir.y;
//             } // +X
//             else
//             {
//                 face = 1;
//                 sc = dir.z;
//                 tc = -dir.y;
//             } // -X
//         }
//         else if (ay >= az)
//         {
//             ma = ay;
//             if (dir.y > 0)
//             {
//                 face = 2;
//                 sc = dir.x;
//                 tc = dir.z;
//             } // +Y
//             else
//             {
//                 face = 3;
//                 sc = dir.x;
//                 tc = -dir.z;
//             } // -Y
//         }
//         else
//         {
//             ma = az;
//             if (dir.z > 0)
//             {
//                 face = 4;
//                 sc = dir.x;
//                 tc = -dir.y;
//             } // +Z
//             else
//             {
//                 face = 5;
//                 sc = -dir.x;
//                 tc = -dir.y;
//             } // -Z
//         }

//         float u = (sc / ma + 1.0f) * 0.5f * static_cast<float>(faceSize - 1);
//         float v = (tc / ma + 1.0f) * 0.5f * static_cast<float>(faceSize - 1);

//         int x0 = static_cast<int>(u), y0 = static_cast<int>(v);
//         int x1 = std::min(x0 + 1, (int)faceSize - 1);
//         int y1 = std::min(y0 + 1, (int)faceSize - 1);
//         float fx = u - x0, fy = v - y0;

//         const uint32_t facePixels = faceSize * faceSize;
//         const float *base = srcData + face * facePixels * 4;

//         auto px = [&](int x, int y) -> const float *
//         {
//             return base + (y * faceSize + x) * 4;
//         };

//         const float *s00 = px(x0, y0), *s10 = px(x1, y0);
//         const float *s01 = px(x0, y1), *s11 = px(x1, y1);

//         glm::vec3 result(0.0f);
//         for (int c = 0; c < 3; c++)
//         {
//             result[c] = s00[c] * (1 - fx) * (1 - fy) + s10[c] * fx * (1 - fy) + s01[c] * (1 - fx) * fy + s11[c] * fx * fy;
//         }
//         return result;
//     }

//     static float RadicalInverseVdC(uint32_t bits)
//     {
//         bits = (bits << 16u) | (bits >> 16u);
//         bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
//         bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
//         bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
//         bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
//         return static_cast<float>(bits) * 2.3283064365386963e-10f;
//     }

//     // Hammersley low-discrepancy sequence
//     static glm::vec2 Hammersley(uint32_t i, uint32_t N)
//     {
//         return {static_cast<float>(i) / static_cast<float>(N), RadicalInverseVdC(i)};
//     }

//     // GGX importance sampling
//     static glm::vec3 ImportanceSampleGGX(glm::vec2 Xi, float roughness)
//     {
//         float a = roughness * roughness;
//         float phi = 2.0f * glm::pi<float>() * Xi.x;
//         float cosTheta = std::sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
//         float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

//         return glm::vec3(
//             std::cos(phi) * sinTheta,
//             std::sin(phi) * sinTheta,
//             cosTheta);
//     }

//     static glm::vec3 TangentToWorld(glm::vec3 H, glm::vec3 N)
//     {
//         glm::vec3 up = std::abs(N.z) < 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
//         glm::vec3 tangent = glm::normalize(glm::cross(up, N));
//         glm::vec3 bitangent = glm::cross(N, tangent);
//         return glm::normalize(tangent * H.x + bitangent * H.y + N * H.z);
//     }

//     static float GeometrySchlickGGX_IBL(float NdotV, float roughness)
//     {
//         float a = roughness;
//         float k = (a * a) / 2.0f;
//         return NdotV / (NdotV * (1.0f - k) + k);
//     }

//     static float GeometrySmith_IBL(float NdotV, float NdotL, float roughness)
//     {
//         return GeometrySchlickGGX_IBL(NdotV, roughness) * GeometrySchlickGGX_IBL(NdotL, roughness);
//     }

//     static std::vector<float> ReadbackCubemapToFloat(
//         const Texture &envMap,
//         const VulkanAllocator &allocator,
//         const VulkanImmediateSubmit &submit)
//     {
//         const VulkanImage &img = envMap.GetImage();
//         const uint32_t faceSize = img.GetWidth();
//         const uint32_t layers = img.GetArrayLayers(); // 6
//         const VkDeviceSize faceBytes = static_cast<VkDeviceSize>(faceSize) * faceSize * 4 * sizeof(float);
//         const VkDeviceSize totalBytes = layers * faceBytes;

//         VulkanBuffer readback(allocator, MakeReadbackBufferDesc(totalBytes));

//         submit.Submit([&](VkCommandBuffer cmd)
//                       {
//             VkImageMemoryBarrier toSrc{
//                 .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
//                 .srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
//                 .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
//                 .oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//                 .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//                 .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//                 .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//                 .image = img.GetVkHandle(),
//                 .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layers},
//             };
//             vkCmdPipelineBarrier(cmd,
//                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//                 VK_PIPELINE_STAGE_TRANSFER_BIT,
//                 0, 0, nullptr, 0, nullptr, 1, &toSrc);

//             for (uint32_t face = 0; face < layers; face++)
//             {
//                 VkBufferImageCopy region{
//                     .bufferOffset = face * faceBytes,
//                     .bufferRowLength = 0,
//                     .bufferImageHeight = 0,
//                     .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, face, 1},
//                     .imageOffset = {0, 0, 0},
//                     .imageExtent = {faceSize, faceSize, 1},
//                 };
//                 vkCmdCopyImageToBuffer(cmd,
//                     img.GetVkHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//                     readback.GetVkHandle(), 1, &region);
//             }

//             VkImageMemoryBarrier toRead{
//                 .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
//                 .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
//                 .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
//                 .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//                 .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//                 .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//                 .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//                 .image = img.GetVkHandle(),
//                 .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layers},
//             };
//             vkCmdPipelineBarrier(cmd,
//                 VK_PIPELINE_STAGE_TRANSFER_BIT,
//                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//                 0, 0, nullptr, 0, nullptr, 1, &toRead); });

//         allocator.InvalidateAllocation(readback.GetAllocation());
//         std::vector<float> data(totalBytes / sizeof(float));
//         std::memcpy(data.data(), readback.GetMappedPtr(), totalBytes);

//         return data;
//     }

//     static std::shared_ptr<Texture> UploadCubemap(
//         const float *data,
//         uint32_t faceSize,
//         uint32_t mipLevels,
//         const VulkanAllocator &allocator,
//         const VulkanLogicalDevice &device,
//         const VulkanImmediateSubmit &submit)
//     {
//         const VkDeviceSize faceBytes = static_cast<VkDeviceSize>(faceSize) * faceSize * 4 * sizeof(float);
//         const VkDeviceSize totalBytes = 6 * faceBytes;

//         VulkanBuffer staging(allocator, MakeStagingBufferDesc(totalBytes));
//         staging.Upload(data, totalBytes);

//         ImageDesc imageDesc{
//             .width = faceSize,
//             .height = faceSize,
//             .mipLevels = mipLevels,
//             .arrayLayers = 6,
//             .format = VK_FORMAT_R32G32B32A32_SFLOAT,
//             .type = ImageType::TextureCube,
//         };

//         auto tex = std::make_shared<Texture>();
//         tex->GetImage();

//         struct TextureAccessor : public Texture
//         {
//             void Init(uint32_t w, uint32_t h, uint32_t mips,
//                       const VulkanAllocator &alloc, const VulkanLogicalDevice &dev)
//             {
//                 m_Width = w;
//                 m_Height = h;
//                 m_MipLevels = mips;
//                 m_Image = std::make_unique<VulkanImage>(alloc, dev, ImageDesc{
//                                                                         .width = w,
//                                                                         .height = h,
//                                                                         .mipLevels = mips,
//                                                                         .arrayLayers = 6,
//                                                                         .format = VK_FORMAT_R32G32B32A32_SFLOAT,
//                                                                         .type = ImageType::TextureCube,
//                                                                     });
//             }
//         };

//         auto *acc = new TextureAccessor();
//         acc->Init(faceSize, faceSize, mipLevels, allocator, device);
//         tex.reset(acc);

//         submit.Submit([&](VkCommandBuffer cmd)
//                       {
//             tex->GetImage().TransitionToTransferDst(cmd);
//             tex->GetImage().CopyFromBufferAllLayers(cmd, staging.GetVkHandle(), faceBytes);
//             tex->GetImage().TransitionToShaderRead(cmd); });

//         tex->GetImage().CreateSampler(SamplerDesc{
//             .addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
//         });

//         return tex;
//     }

//     static std::shared_ptr<Texture> BakeIrradiance(
//         const float *envData,
//         uint32_t envFaceSize,
//         const VulkanAllocator &allocator,
//         const VulkanLogicalDevice &device,
//         const VulkanImmediateSubmit &submit)
//     {
//         constexpr uint32_t kSize = 32;
//         constexpr float kInvN = 1.0f / static_cast<float>(kSize);
//         constexpr uint32_t kSamples = 64;
//         constexpr float kDelta = glm::pi<float>() / static_cast<float>(kSamples);
//         constexpr float kInv2Pi = 1.0f / (2.0f * glm::pi<float>());

//         const uint32_t pixPerFace = kSize * kSize;
//         std::vector<float> out(6 * pixPerFace * 4, 0.0f);

//         std::vector<std::future<void>> jobs;
//         jobs.reserve(6);

//         for (uint32_t face = 0; face < 6; face++)
//         {
//             jobs.push_back(std::async(std::launch::async, [&, face]()
//                                       {
//                 const FaceAxes &ax = k_Faces[face];
//                 float *facePtr = out.data() + face * pixPerFace * 4;

//                 for (uint32_t y = 0; y < kSize; y++)
//                 {
//                     const float fv = (2.0f * (y + 0.5f) * kInvN) - 1.0f;

//                     for (uint32_t x = 0; x < kSize; x++)
//                     {
//                         const float fu = (2.0f * (x + 0.5f) * kInvN) - 1.0f;

//                         glm::vec3 N = glm::normalize(ax.dir + fu * ax.right + fv * ax.up);

//                         glm::vec3 up      = std::abs(N.z) < 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
//                         glm::vec3 right   = glm::normalize(glm::cross(up, N));
//                         glm::vec3 forward = glm::cross(N, right);

//                         glm::vec3 irradiance(0.0f);
//                         float weight = 0.0f;

//                         for (uint32_t phiIdx = 0; phiIdx < kSamples * 2; phiIdx++)
//                         {
//                             float phi = kDelta * phiIdx;
//                             float sinPhi = std::sin(phi), cosPhi = std::cos(phi);

//                             for (uint32_t thetaIdx = 0; thetaIdx < kSamples / 2; thetaIdx++)
//                             {
//                                 float theta    = kDelta * thetaIdx;
//                                 float sinTheta = std::sin(theta), cosTheta = std::cos(theta);

//                                 glm::vec3 sampleDir =
//                                     sinTheta * cosPhi * right +
//                                     sinTheta * sinPhi * forward +
//                                     cosTheta * N;

//                                 float NdotL = std::max(cosTheta, 0.0f);

//                                 irradiance += SampleCubemapCPU(envData, envFaceSize, sampleDir)
//                                             * NdotL * sinTheta;
//                                 weight += sinTheta;
//                             }
//                         }

//                         irradiance = glm::pi<float>() * irradiance / weight;

//                         float *dst = facePtr + (y * kSize + x) * 4;
//                         dst[0] = irradiance.r;
//                         dst[1] = irradiance.g;
//                         dst[2] = irradiance.b;
//                         dst[3] = 1.0f;
//                     }
//                 } }));
//         }

//         for (auto &j : jobs)
//             j.get();

//         return UploadCubemap(out.data(), kSize, 1, allocator, device, submit);
//     }

//     static std::shared_ptr<Texture> BakePrefiltered(
//         const float *envData,
//         uint32_t envFaceSize,
//         const VulkanAllocator &allocator,
//         const VulkanLogicalDevice &device,
//         const VulkanImmediateSubmit &submit)
//     {
//         constexpr uint32_t kBaseSize = 128;
//         constexpr uint32_t kMipLevels = 5;
//         constexpr uint32_t kSamples = 1024;

//         struct MipData
//         {
//             uint32_t size;
//             float roughness;
//             std::vector<float> pixels; // 6 * size * size * 4 floats
//         };

//         std::array<MipData, kMipLevels> mips;
//         for (uint32_t m = 0; m < kMipLevels; m++)
//         {
//             mips[m].size = std::max(1u, kBaseSize >> m);
//             mips[m].roughness = static_cast<float>(m) / static_cast<float>(kMipLevels - 1);
//             mips[m].pixels.resize(6 * mips[m].size * mips[m].size * 4, 0.0f);
//         }

//         std::vector<std::future<void>> jobs;

//         for (uint32_t m = 0; m < kMipLevels; m++)
//         {
//             for (uint32_t face = 0; face < 6; face++)
//             {
//                 jobs.push_back(std::async(std::launch::async, [&, m, face]()
//                                           {
//                     const uint32_t N    = mips[m].size;
//                     const float roughness = mips[m].roughness;
//                     const float invN    = 1.0f / static_cast<float>(N);
//                     const FaceAxes &ax  = k_Faces[face];
//                     float *facePtr = mips[m].pixels.data() + face * N * N * 4;

//                     for (uint32_t y = 0; y < N; y++)
//                     {
//                         const float fv = (2.0f * (y + 0.5f) * invN) - 1.0f;

//                         for (uint32_t x = 0; x < N; x++)
//                         {
//                             const float fu = (2.0f * (x + 0.5f) * invN) - 1.0f;

//                             glm::vec3 R = glm::normalize(ax.dir + fu * ax.right + fv * ax.up);
//                             glm::vec3 Nvec = R;

//                             glm::vec3 prefilteredColor(0.0f);
//                             float totalWeight = 0.0f;

//                             for (uint32_t i = 0; i < kSamples; i++)
//                             {
//                                 glm::vec2 Xi = Hammersley(i, kSamples);
//                                 glm::vec3 H  = TangentToWorld(ImportanceSampleGGX(Xi, roughness), Nvec);
//                                 glm::vec3 L  = glm::normalize(2.0f * glm::dot(Nvec, H) * H - Nvec);

//                                 float NdotL = std::max(glm::dot(Nvec, L), 0.0f);
//                                 if (NdotL > 0.0f)
//                                 {
//                                     prefilteredColor += SampleCubemapCPU(envData, envFaceSize, L) * NdotL;
//                                     totalWeight      += NdotL;
//                                 }
//                             }

//                             if (totalWeight > 0.0f)
//                                 prefilteredColor /= totalWeight;

//                             float *dst = facePtr + (y * N + x) * 4;
//                             dst[0] = prefilteredColor.r;
//                             dst[1] = prefilteredColor.g;
//                             dst[2] = prefilteredColor.b;
//                             dst[3] = 1.0f;
//                         }
//                     } }));
//             }
//         }

//         for (auto &j : jobs)
//             j.get();

//         struct PrefilteredTexture : public Texture
//         {
//             void Init(uint32_t w, uint32_t mips,
//                       const VulkanAllocator &alloc, const VulkanLogicalDevice &dev)
//             {
//                 m_Width = w;
//                 m_Height = w;
//                 m_MipLevels = mips;
//                 m_Image = std::make_unique<VulkanImage>(alloc, dev, ImageDesc{
//                                                                         .width = w,
//                                                                         .height = w,
//                                                                         .mipLevels = mips,
//                                                                         .arrayLayers = 6,
//                                                                         .format = VK_FORMAT_R32G32B32A32_SFLOAT,
//                                                                         .type = ImageType::TextureCube,
//                                                                     });
//             }
//         };

//         auto *raw = new PrefilteredTexture();
//         raw->Init(kBaseSize, kMipLevels, allocator, device);
//         auto tex = std::shared_ptr<Texture>(raw);

//         submit.Submit([&](VkCommandBuffer cmd)
//                       { tex->GetImage().TransitionLayout(
//                             cmd,
//                             VK_IMAGE_LAYOUT_UNDEFINED,
//                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
//                             VK_PIPELINE_STAGE_TRANSFER_BIT,
//                             0,
//                             VK_ACCESS_TRANSFER_WRITE_BIT); });

//         for (uint32_t m = 0; m < kMipLevels; m++)
//         {
//             const uint32_t N = mips[m].size;
//             const VkDeviceSize faceBytes = static_cast<VkDeviceSize>(N) * N * 4 * sizeof(float);
//             const VkDeviceSize totalBytes = 6 * faceBytes;

//             VulkanBuffer staging(allocator, MakeStagingBufferDesc(totalBytes));
//             staging.Upload(mips[m].pixels.data(), totalBytes);

//             mips[m].pixels.clear();
//             mips[m].pixels.shrink_to_fit();

//             submit.Submit([&, m, N, faceBytes](VkCommandBuffer cmd)
//                           {
//                 for (uint32_t face = 0; face < 6; face++)
//                 {
//                     VkBufferImageCopy region{
//                         .bufferOffset      = face * faceBytes,
//                         .bufferRowLength   = 0,
//                         .bufferImageHeight = 0,
//                         .imageSubresource  = {VK_IMAGE_ASPECT_COLOR_BIT, m, face, 1},
//                         .imageOffset       = {0, 0, 0},
//                         .imageExtent       = {N, N, 1},
//                     };
//                     vkCmdCopyBufferToImage(
//                         cmd,
//                         staging.GetVkHandle(),
//                         tex->GetImage().GetVkHandle(),
//                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                         1, &region);
//                 } });
//         }

//         submit.Submit([&](VkCommandBuffer cmd)
//                       { tex->GetImage().TransitionLayout(
//                             cmd,
//                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//                             VK_PIPELINE_STAGE_TRANSFER_BIT,
//                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//                             VK_ACCESS_TRANSFER_WRITE_BIT,
//                             VK_ACCESS_SHADER_READ_BIT); });

//         tex->GetImage().CreateSampler(SamplerDesc{
//             .addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
//         });

//         return tex;
//     }

//     static std::shared_ptr<Texture> BakeBrdfLUT(
//         const VulkanAllocator &allocator,
//         const VulkanLogicalDevice &device,
//         const VulkanImmediateSubmit &submit)
//     {
//         constexpr uint32_t kSize = 512;
//         constexpr uint32_t kSamples = 1024;

//         std::vector<float> data(kSize * kSize * 4, 0.0f); // RGBA32F

//         std::vector<std::future<void>> jobs;
//         jobs.reserve(kSize);

//         for (uint32_t y = 0; y < kSize; y++)
//         {
//             jobs.push_back(std::async(std::launch::async, [&, y]()
//                                       {
//                 float NdotV    = (y + 0.5f) / static_cast<float>(kSize);
//                 float roughness = 0.0f;

//                 glm::vec3 V(std::sqrt(1.0f - NdotV * NdotV), 0.0f, NdotV); // sin, 0, cos

//                 for (uint32_t x = 0; x < kSize; x++)
//                 {
//                     roughness = (x + 0.5f) / static_cast<float>(kSize);

//                     float A = 0.0f, B = 0.0f;

//                     for (uint32_t i = 0; i < kSamples; i++)
//                     {
//                         glm::vec2 Xi = Hammersley(i, kSamples);
//                         glm::vec3 N  = glm::vec3(0, 0, 1);
//                         glm::vec3 H  = TangentToWorld(ImportanceSampleGGX(Xi, roughness), N);
//                         glm::vec3 L  = glm::normalize(2.0f * glm::dot(V, H) * H - V);

//                         float NdotL_ = std::max(L.z, 0.0f);
//                         float NdotH  = std::max(H.z, 0.0f);
//                         float VdotH  = std::max(glm::dot(V, H), 0.0f);

//                         if (NdotL_ > 0.0f)
//                         {
//                             float G     = GeometrySmith_IBL(NdotV, NdotL_, roughness);
//                             float G_Vis = (G * VdotH) / (NdotH * NdotV + 1e-6f);
//                             float Fc    = std::pow(1.0f - VdotH, 5.0f);

//                             A += (1.0f - Fc) * G_Vis;
//                             B += Fc * G_Vis;
//                         }
//                     }

//                     A /= static_cast<float>(kSamples);
//                     B /= static_cast<float>(kSamples);

//                     float *dst = data.data() + (y * kSize + x) * 4;
//                     dst[0] = A;   // scale
//                     dst[1] = B;   // bias
//                     dst[2] = 0.0f;
//                     dst[3] = 1.0f;
//                 } }));
//         }

//         for (auto &j : jobs)
//             j.get();

//         VkDeviceSize dataSize = static_cast<VkDeviceSize>(kSize) * kSize * 4 * sizeof(float);
//         VulkanBuffer staging(allocator, MakeStagingBufferDesc(dataSize));
//         staging.Upload(data.data(), dataSize);

//         struct BrdfTexture : public Texture
//         {
//             void Init(uint32_t size, const VulkanAllocator &alloc, const VulkanLogicalDevice &dev)
//             {
//                 m_Width = size;
//                 m_Height = size;
//                 m_MipLevels = 1;
//                 m_Image = std::make_unique<VulkanImage>(alloc, dev, ImageDesc{
//                                                                         .width = size,
//                                                                         .height = size,
//                                                                         .mipLevels = 1,
//                                                                         .format = VK_FORMAT_R32G32B32A32_SFLOAT,
//                                                                         .type = ImageType::Texture2D,
//                                                                     });
//             }
//         };

//         auto *raw = new BrdfTexture();
//         raw->Init(kSize, allocator, device);
//         auto tex = std::shared_ptr<Texture>(raw);

//         submit.Submit([&](VkCommandBuffer cmd)
//                       {
//             tex->GetImage().TransitionToTransferDst(cmd);
//             tex->GetImage().CopyFromBuffer(cmd, staging.GetVkHandle());
//             tex->GetImage().TransitionToShaderRead(cmd); });

//         tex->GetImage().CreateSampler(SamplerDesc{
//             .addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
//         });

//         return tex;
//     }

//     IBLBaker::Result IBLBaker::Bake(
//         const Texture &envMap,
//         const VulkanAllocator &allocator,
//         const VulkanLogicalDevice &device,
//         const VulkanImmediateSubmit &submit)
//     {
//         Timer timer;
//         VE_CORE_INFO("IBLBaker: starting bake...");

//         Timer readbackTimer;
//         std::vector<float> envData = ReadbackCubemapToFloat(envMap, allocator, submit);
//         const uint32_t envFaceSize = envMap.GetWidth();
//         VE_CORE_INFO("IBLBaker: cubemap readback {}x{} ({} ms)", envFaceSize, envFaceSize, readbackTimer.ElapsedMilliseconds());

//         Result result;

//         // 1. Irradiance
//         {
//             Timer t;
//             result.irradianceMap = BakeIrradiance(envData.data(), envFaceSize, allocator, device, submit);
//             VE_CORE_INFO("IBLBaker: irradiance 32x32 done ({} ms)", t.ElapsedMilliseconds());
//         }

//         // 2. Prefiltered
//         {
//             Timer t;
//             result.prefilteredMap = BakePrefiltered(envData.data(), envFaceSize, allocator, device, submit);
//             VE_CORE_INFO("IBLBaker: prefiltered 128x128 x5 mips done ({} ms)", t.ElapsedMilliseconds());
//         }

//         // 3. BRDF LUT
//         {
//             Timer t;
//             result.brdfLUT = BakeBrdfLUT(allocator, device, submit);
//             VE_CORE_INFO("IBLBaker: BRDF LUT 512x512 done ({} ms)", t.ElapsedMilliseconds());
//         }

//         VE_CORE_INFO("IBLBaker: total bake time {} ms", timer.ElapsedMilliseconds());
//         return result;
//     }

// } // namespace ve
