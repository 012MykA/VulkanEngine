#pragma once

#include "Backends/Vulkan/VulkanImage.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>
#include <memory>

namespace ve
{
    enum class TextureFormat
    {
        RGBA8_SRGB,
        RGBA8_UNORM,
        RGBA16_SFLOAT,
        RGBA32_SFLOAT,
        RG8_UNORM,
        R8_UNORM,
    };

    struct TextureDesc
    {
        const void *pixels = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        TextureFormat format = TextureFormat::RGBA8_SRGB;
        bool generateMips = true;
    };

    class VulkanAllocator;
    class VulkanLogicalDevice;
    class VulkanImmediateSubmit;

    class Texture
    {
    public:
        Texture(const TextureDesc &desc);
        ~Texture() = default;

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;

    public:
        // Upload data on GPU
        void Upload(const VulkanAllocator &allocator,
                    const VulkanLogicalDevice &device,
                    const VulkanImmediateSubmit &upload,
                    const SamplerDesc &sampleDesc);
        void FreeCPUData();

        // --- Setters ---

        void SetData(const void *data, uint32_t width, uint32_t height, TextureFormat format);
        void SetGenerateMips(bool generate);

        // --- Getters ---

        bool IsUploaded() const { return m_Image != nullptr; }
        const VulkanImage &GetImage() { return *m_Image; }
        VkDescriptorImageInfo GetDescriptorInfo() const { return m_Image->GetDescriptorInfo(); }
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        bool IsMipsGenerated() const { return m_GenerateMips; }

    private:
        static VkFormat ResolveVkFormat(TextureFormat format);
        static size_t GetPixelSize(TextureFormat format);
        static uint32_t CalcMipLevels(uint32_t w, uint32_t h);

    private:
        std::vector<uint8_t> m_CPUData;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        TextureFormat m_Format = TextureFormat::RGBA8_SRGB;
        bool m_GenerateMips = true;

        std::unique_ptr<VulkanImage> m_Image;
        uint32_t m_MipLevels = 1;
    };

} // namespace ve
