#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <span>
#include <cstdint>
#include <initializer_list>
#include <deque>

namespace ve
{
    class VulkanLogicalDevice;

    // --- VulkanDescriptorSetLayout ---
    class VulkanDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            explicit Builder(const VulkanLogicalDevice &logicalDevice);

            Builder &AddBinding(uint32_t binding,
                                VkDescriptorType type,
                                VkShaderStageFlags stages,
                                uint32_t count = 1);

            Builder &AddImmutableSamplerBinding(uint32_t binding,
                                                VkShaderStageFlags stages,
                                                std::vector<VkSampler> samplers);

            [[nodiscard]] VulkanDescriptorSetLayout Build() const;

        private:
            VkDevice m_Device;
            std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
            std::vector<std::vector<VkSampler>> m_ImmutableSamplers;
        };

    public:
        VulkanDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout);
        ~VulkanDescriptorSetLayout();

        VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout &) = delete;
        VulkanDescriptorSetLayout &operator=(const VulkanDescriptorSetLayout &) = delete;

        VulkanDescriptorSetLayout(VulkanDescriptorSetLayout &&other) noexcept;
        VulkanDescriptorSetLayout &operator=(VulkanDescriptorSetLayout &&other) noexcept;

        VkDescriptorSetLayout GetVkHandle() const { return m_Layout; }

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
    };

    // --- VulkanDescriptorPool ---
    struct DescriptorPoolDesc
    {
        uint32_t maxSets = 100;
        std::vector<VkDescriptorPoolSize> poolSizes;

        bool allowFreeDescriptorSet = false;
    };

    class VulkanDescriptorPool
    {
    public:
        explicit VulkanDescriptorPool(const VulkanLogicalDevice &logicalDevice, const DescriptorPoolDesc &desc = {});
        ~VulkanDescriptorPool();

        VulkanDescriptorPool(const VulkanDescriptorPool &) = delete;
        VulkanDescriptorPool &operator=(const VulkanDescriptorPool &) = delete;

        [[nodiscard]] VkDescriptorSet Allocate(VkDescriptorSetLayout layout) const;

        [[nodiscard]] std::vector<VkDescriptorSet> AllocateMany(VkDescriptorSetLayout layout, uint32_t count) const;

        void Free(VkDescriptorSet set) const;

        void Reset() const;

        VkDescriptorPool GetVkHandle() const { return m_Pool; }

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VkDescriptorPool m_Pool = VK_NULL_HANDLE;
        bool m_AllowFree = false;
    };

    // --- VulkanDescriptorWriter ---
    class VulkanDescriptorWriter
    {
    public:
        VulkanDescriptorWriter(VkDevice device);

        VulkanDescriptorWriter &WriteBuffer(
            uint32_t binding,
            VkDescriptorType type,
            VkDescriptorSet set,
            const VkDescriptorBufferInfo &bufferInfo);

        VulkanDescriptorWriter &WriteImage(
            uint32_t binding,
            VkDescriptorType type,
            VkDescriptorSet set,
            const VkDescriptorImageInfo &imageInfo);

        VulkanDescriptorWriter &WriteImageArray(
            uint32_t binding,
            VkDescriptorType type,
            VkDescriptorSet set,
            std::span<const VkDescriptorImageInfo> imageInfos);

        void Flush();

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        std::vector<VkWriteDescriptorSet> m_Writes;
        std::deque<VkDescriptorBufferInfo> m_BufferInfos;
        std::deque<VkDescriptorImageInfo> m_ImageInfos;
    };

} // namespace ve
