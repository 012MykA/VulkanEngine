#pragma once

#include <vulkan/vulkan.h>

namespace ve
{
    struct SamplerConfig
    {
        VkFilter MagFilter = VK_FILTER_LINEAR;
        VkFilter MinFilter = VK_FILTER_LINEAR;
        VkSamplerAddressMode AddressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode AddressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode AddressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkBool32 AnisotropyEnable = VK_FALSE;
        float MaxAnisotropy = 1.0f;
        VkBorderColor BorderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        VkBool32 UnnormalizedCoordinates = VK_FALSE;
        VkBool32 CompareEnable = VK_FALSE;
        VkCompareOp CompareOp = VK_COMPARE_OP_ALWAYS;
        VkSamplerMipmapMode MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        float MinLod = 0.0f;
        float MaxLod = 0.0f;
        float MipLodBias = 0.0f;
    };

    class VulkanSampler
    {
    public:
        VulkanSampler(VkDevice device, const SamplerConfig &config);
        ~VulkanSampler();

        VkSampler GetSampler() const { return m_Sampler; }

    private:
        VkDevice m_Device;
        VkSampler m_Sampler = VK_NULL_HANDLE;
    };

} // namespace ve
