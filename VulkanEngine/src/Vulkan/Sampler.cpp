#include "Sampler.hpp"
#include "Device.hpp"
#include "Validation.hpp"

namespace VE
{
    Sampler::Sampler(const Device &device, const SamplerConfig &config) : m_Device(device)
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_Device.GetPhysicalDevice(), &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = config.MagFilter;
        samplerInfo.minFilter = config.MinFilter;
        samplerInfo.addressModeU = config.AddressModeU;
        samplerInfo.addressModeV = config.AddressModeV;
        samplerInfo.addressModeW = config.AddressModeW;
        samplerInfo.anisotropyEnable = config.AnisotropyEnable;
        if (config.AnisotropyEnable)
        {
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        }
        else
        {
            samplerInfo.maxAnisotropy = 1.0;
        }
        samplerInfo.borderColor = config.BorderColor;
        samplerInfo.unnormalizedCoordinates = config.UnnormalizedCoordinates;
        samplerInfo.compareEnable = config.CompareEnable;
        samplerInfo.compareOp = config.CompareOp;
        samplerInfo.mipmapMode = config.MipmapMode;
        samplerInfo.mipLodBias = config.MipLodBias;
        samplerInfo.minLod = config.MinLod;
        samplerInfo.maxLod = config.MaxLod;

        CheckVk(vkCreateSampler(m_Device.Handle(), &samplerInfo, nullptr, &m_Sampler), "create sampler!");
    }

    Sampler::~Sampler()
    {
        vkDestroySampler(m_Device.Handle(), m_Sampler, nullptr);
    }

}
