#include "VulkanSampler.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    VulkanSampler::VulkanSampler(VkDevice device, const SamplerConfig &config) : m_Device(device)
    {
        VkSamplerCreateInfo samplerInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = config.MagFilter,
            .minFilter = config.MinFilter,
            .mipmapMode = config.MipmapMode,
            .addressModeU = config.AddressModeU,
            .addressModeV = config.AddressModeV,
            .addressModeW = config.AddressModeW,
            .mipLodBias = config.MipLodBias,
            .anisotropyEnable = config.AnisotropyEnable,
            .maxAnisotropy = config.MaxAnisotropy,
            .compareEnable = config.CompareEnable,
            .compareOp = config.CompareOp,
            .minLod = config.MinLod,
            .maxLod = config.MaxLod,
            .borderColor = config.BorderColor,
            .unnormalizedCoordinates = config.UnnormalizedCoordinates,
        };

        VkResult result = vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler);
        CHECK_VK_RESULT(result);
    }

    VulkanSampler::~VulkanSampler()
    {
        vkDestroySampler(m_Device, m_Sampler, nullptr);
    }

} // namespace ve
