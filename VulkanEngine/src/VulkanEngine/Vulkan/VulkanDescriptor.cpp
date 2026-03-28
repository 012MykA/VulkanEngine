#include "VulkanDescriptor.hpp"
#include "VulkanLogicalDevice.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    // --- Builder ---
    VulkanDescriptorSetLayout::Builder::Builder(const VulkanLogicalDevice &logicalDevice)
        : m_Device(logicalDevice.GetVkHandle())
    {
    }

    VulkanDescriptorSetLayout::Builder &VulkanDescriptorSetLayout::Builder::AddBinding(
        uint32_t binding,
        VkDescriptorType type,
        VkShaderStageFlags stages,
        uint32_t count)
    {
        m_Bindings.push_back(VkDescriptorSetLayoutBinding{
            .binding = binding,
            .descriptorType = type,
            .descriptorCount = count,
            .stageFlags = stages,
            .pImmutableSamplers = nullptr,
        });
        return *this;
    }

    VulkanDescriptorSetLayout::Builder &VulkanDescriptorSetLayout::Builder::AddImmutableSamplerBinding(
        uint32_t binding,
        VkShaderStageFlags stages,
        std::vector<VkSampler> samplers)
    {
        m_ImmutableSamplers.push_back(std::move(samplers));
        const auto &stored = m_ImmutableSamplers.back();

        m_Bindings.push_back(VkDescriptorSetLayoutBinding{
            .binding = binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = static_cast<uint32_t>(stored.size()),
            .stageFlags = stages,
            .pImmutableSamplers = stored.data(),
        });
        return *this;
    }

    VulkanDescriptorSetLayout VulkanDescriptorSetLayout::Builder::Build() const
    {
        VkDescriptorSetLayoutCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(m_Bindings.size()),
            .pBindings = m_Bindings.data(),
        };

        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VkResult result = vkCreateDescriptorSetLayout(m_Device, &createInfo, nullptr, &layout);
        CHECK_VK_RESULT(result);

        return VulkanDescriptorSetLayout(m_Device, layout);
    }

    // --- VulkanDescriptorSetLayout ---

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout)
        : m_Device(device), m_Layout(layout)
    {
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
    {
        if (m_Layout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(m_Device, m_Layout, nullptr);
    }

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDescriptorSetLayout &&other) noexcept
        : m_Device(other.m_Device), m_Layout(other.m_Layout)
    {
        other.m_Layout = VK_NULL_HANDLE;
    }

    VulkanDescriptorSetLayout &VulkanDescriptorSetLayout::operator=(VulkanDescriptorSetLayout &&other) noexcept
    {
        if (this != &other)
        {
            if (m_Layout != VK_NULL_HANDLE)
                vkDestroyDescriptorSetLayout(m_Device, m_Layout, nullptr);
            m_Device = other.m_Device;
            m_Layout = other.m_Layout;
            other.m_Layout = VK_NULL_HANDLE;
        }
        return *this;
    }

    // --- VulkanDescriptorPool ---
    VulkanDescriptorPool::VulkanDescriptorPool(const VulkanLogicalDevice &logicalDevice, const DescriptorPoolDesc &desc)
        : m_Device(logicalDevice.GetVkHandle()), m_AllowFree(desc.allowFreeDescriptorSet)
    {
        std::vector<VkDescriptorPoolSize> sizes = desc.poolSizes;

        // Default values for PBR rendering ~100 objects
        if (sizes.empty())
        {
            sizes = {
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, desc.maxSets * 2},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, desc.maxSets},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, desc.maxSets * 8},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, desc.maxSets * 4},
                {VK_DESCRIPTOR_TYPE_SAMPLER, desc.maxSets},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, desc.maxSets},
            };
        }

        VkDescriptorPoolCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = desc.allowFreeDescriptorSet ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : VkDescriptorPoolCreateFlags(0),
            .maxSets = desc.maxSets,
            .poolSizeCount = static_cast<uint32_t>(sizes.size()),
            .pPoolSizes = sizes.data(),
        };

        VkResult result = vkCreateDescriptorPool(m_Device, &createInfo, nullptr, &m_Pool);
        CHECK_VK_RESULT(result);
        VE_CORE_TRACE("VulkanDescriptorPool created (maxSets={})", desc.maxSets);
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        if (m_Pool != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(m_Device, m_Pool, nullptr);
    }

    VkDescriptorSet VulkanDescriptorPool::Allocate(VkDescriptorSetLayout layout) const
    {
        VkDescriptorSetAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_Pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &layout,
        };

        VkDescriptorSet set = VK_NULL_HANDLE;
        VkResult result = vkAllocateDescriptorSets(m_Device, &allocInfo, &set);
        CHECK_VK_RESULT(result);
        return set;
    }

    std::vector<VkDescriptorSet> VulkanDescriptorPool::AllocateMany(VkDescriptorSetLayout layout, uint32_t count) const
    {
        std::vector<VkDescriptorSetLayout> layouts(count, layout);

        VkDescriptorSetAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_Pool,
            .descriptorSetCount = count,
            .pSetLayouts = layouts.data(),
        };

        std::vector<VkDescriptorSet> sets(count);
        VkResult result = vkAllocateDescriptorSets(m_Device, &allocInfo, sets.data());
        CHECK_VK_RESULT(result);
        return sets;
    }

    void VulkanDescriptorPool::Free(VkDescriptorSet set) const
    {
        if (m_AllowFree)
        {
            VE_CORE_WARN("VulkanDescriptorPool::Free called but allowFreeDescriptorSet=false");
            return;
        }
        vkFreeDescriptorSets(m_Device, m_Pool, 1, &set);
    }

    void VulkanDescriptorPool::Reset() const
    {
        vkResetDescriptorPool(m_Device, m_Pool, 0);
    }

    // --- VulkanDescriptorWriter ---
    VulkanDescriptorWriter::VulkanDescriptorWriter(VkDevice device)
        : m_Device(device)
    {
    }

    VulkanDescriptorWriter &VulkanDescriptorWriter::WriteBuffer(
        uint32_t binding,
        VkDescriptorType type,
        VkDescriptorSet set,
        const VkDescriptorBufferInfo &bufferInfo)
    {
        m_BufferInfos.push_back(bufferInfo);

        m_Writes.push_back(VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = set,
            .dstBinding = binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = type,
            .pBufferInfo = &m_BufferInfos.back(),
        });
        return *this;
    }

    VulkanDescriptorWriter &VulkanDescriptorWriter::WriteImage(
        uint32_t binding,
        VkDescriptorType type,
        VkDescriptorSet set,
        const VkDescriptorImageInfo &imageInfo)
    {
        m_ImageInfos.push_back(imageInfo);

        m_Writes.push_back(VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = set,
            .dstBinding = binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = type,
            .pImageInfo = &m_ImageInfos.back(),
        });
        return *this;
    }

    VulkanDescriptorWriter &VulkanDescriptorWriter::WriteImageArray(
        uint32_t binding,
        VkDescriptorType type,
        VkDescriptorSet set,
        std::span<const VkDescriptorImageInfo> imageInfos)
    {
        const size_t offset = m_ImageInfos.size();
        m_ImageInfos.insert(m_ImageInfos.end(), imageInfos.begin(), imageInfos.end());

        m_Writes.push_back(VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = set,
            .dstBinding = binding,
            .dstArrayElement = 0,
            .descriptorCount = static_cast<uint32_t>(m_ImageInfos.size()),
            .descriptorType = type,
            .pImageInfo = m_ImageInfos.data() + offset,
        });
        return *this;
    }

    void VulkanDescriptorWriter::Flush()
    {
        if (!m_Writes.empty())
        {
            vkUpdateDescriptorSets(
                m_Device,
                static_cast<uint32_t>(m_Writes.size()),
                m_Writes.data(),
                0, nullptr);
        }

        m_Writes.clear();
        m_BufferInfos.clear();
        m_ImageInfos.clear();
    }

} // namespace ve
