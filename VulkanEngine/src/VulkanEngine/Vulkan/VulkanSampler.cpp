#include "VulkanSampler.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    VulkanSampler::VulkanSampler(VkDevice device, const SamplerConfig &config) : m_Device(device)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = config.MagFilter;
        samplerInfo.minFilter = config.MinFilter;
        samplerInfo.addressModeU = config.AddressModeU;
        samplerInfo.addressModeV = config.AddressModeV;
        samplerInfo.addressModeW = config.AddressModeW;
        samplerInfo.anisotropyEnable = config.AnisotropyEnable;
        samplerInfo.maxAnisotropy = config.MaxAnisotropy;
        samplerInfo.borderColor = config.BorderColor;
        samplerInfo.unnormalizedCoordinates = config.UnnormalizedCoordinates;
        samplerInfo.compareEnable = config.CompareEnable;
        samplerInfo.compareOp = config.CompareOp;
        samplerInfo.mipmapMode = config.MipmapMode;
        samplerInfo.mipLodBias = config.MipLodBias;
        samplerInfo.minLod = config.MinLod;
        samplerInfo.maxLod = config.MaxLod;

        VkResult result = vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler);
        CHECK_VK_RESULT(result);
    }

    VulkanSampler::~VulkanSampler()
    {
        vkDestroySampler(m_Device, m_Sampler, nullptr);
    }

} // namespace ve
