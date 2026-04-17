#pragma once

#include "Backends/Vulkan/VulkanImage.hpp"
#include "Backends/Vulkan/VulkanAllocator.hpp"

#include <string>
#include <memory>
#include <cstdint>

namespace ve
{
    class VulkanLogicalDevice;
    class VulkanImmediateSubmit;

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
        TextureFormat format = TextureFormat::RGBA8_SRGB;
        bool generateMips = true;
    };

    class Texture
    {
    public:
        Texture() = default;

        static std::shared_ptr<Texture> LoadFromFile(
            const std::string &path,
            TextureDesc desc,
            const VulkanAllocator &allocator,
            const VulkanLogicalDevice &logicalDevice,
            const VulkanImmediateSubmit &upload);

        // Converts an equirectangular HDR panorama to a TextureCube on the CPU (face-by-face)
        // faceSize is the size of a single face (512, 1024, 2048)
        static std::shared_ptr<Texture> LoadCubemapFromEquirect(
            const std::string &path,
            uint32_t faceSize,
            const VulkanAllocator &allocator,
            const VulkanLogicalDevice &device,
            const VulkanImmediateSubmit &upload);

        static std::shared_ptr<Texture> LoadFromMemory(
            const uint8_t *data,
            size_t size,
            TextureDesc &desc,
            const VulkanAllocator &allocator,
            const VulkanLogicalDevice &device,
            const VulkanImmediateSubmit &upload);

        static std::shared_ptr<Texture> CreateEmpty(
            uint32_t width,
            uint32_t height,
            const TextureDesc &desc,
            const VulkanAllocator &allocator,
            const VulkanLogicalDevice &device);

        static std::shared_ptr<Texture> CreateSolid(
            uint8_t r, uint8_t g, uint8_t b, uint8_t a,
            const VulkanAllocator &allocator,
            const VulkanLogicalDevice &device,
            const VulkanImmediateSubmit &upload);

        VulkanImage &GetImage() { return *m_Image; }
        const VulkanImage &GetImage() const { return *m_Image; }

        VkDescriptorImageInfo GetDescriptorInfo() const { return m_Image->GetDescriptorInfo(); }

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        uint32_t GetMipLevels() const { return m_MipLevels; }
        const std::string &GetPath() const { return m_Path; }

        void InitializeAndUpload(
            const void *pixels,
            uint32_t width,
            uint32_t height,
            const TextureDesc &desc,
            const VulkanAllocator &allocator,
            const VulkanLogicalDevice &logicalDevice,
            const VulkanImmediateSubmit &upload);

    private:
        static VkFormat ResolveVkFormat(TextureFormat format);
        static uint32_t CalcMipLevels(uint32_t w, uint32_t h);
        static size_t GetPixelSize(TextureFormat format);

    protected:
        std::unique_ptr<VulkanImage> m_Image;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_MipLevels = 1;
        std::string m_Path;
    };

} // namespace ve
