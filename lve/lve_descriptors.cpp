//
// Created by Junhao Wang (@forkercat) on 4/22/24.
//

#include "lve_descriptors.h"

namespace lve
{
	/////////////////////////////////////////////////////////////////////////////////
	// Descriptor set layout builder
	/////////////////////////////////////////////////////////////////////////////////
	LveDescriptorSetLayout::Builder& LveDescriptorSetLayout::Builder::AddBinding(
		U32 binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, U32 count)
	{
		ASSERT(m_bindings.count(binding) == 0, "Failed to add binding to builder. The binding already exists!");

		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;
		m_bindings[binding] = layoutBinding;

		return *this;
	}

	UniqueRef<LveDescriptorSetLayout> LveDescriptorSetLayout::Builder::Build() const
	{
		return MakeUniqueRef<LveDescriptorSetLayout>(m_device, m_bindings);
	}

	/////////////////////////////////////////////////////////////////////////////////
	// Descriptor set layout
	/////////////////////////////////////////////////////////////////////////////////

	LveDescriptorSetLayout::LveDescriptorSetLayout(
		LveDevice& device, std::unordered_map<U32, VkDescriptorSetLayoutBinding> bindings)
		: m_device(device), m_bindings(bindings)
	{
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings{};

		for (const auto& kv : bindings)
		{
			layoutBindings.push_back(kv.second);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<U32>(layoutBindings.size());
		layoutInfo.pBindings = layoutBindings.data();

		VkResult result = vkCreateDescriptorSetLayout(
			m_device.GetDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create descriptor set layout!");
	}

	LveDescriptorSetLayout::~LveDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_device.GetDevice(), m_descriptorSetLayout, nullptr);
	}

	/////////////////////////////////////////////////////////////////////////////////
	// Descriptor pool builder
	/////////////////////////////////////////////////////////////////////////////////

	LveDescriptorPool::Builder& LveDescriptorPool::Builder::AddPoolSize(VkDescriptorType descriptorType, U32 descriptorCount)
	{
		m_poolSizes.push_back({ descriptorType, descriptorCount });
		return *this;
	}

	LveDescriptorPool::Builder& LveDescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags)
	{
		m_poolFlags = flags;
		return *this;
	}

	LveDescriptorPool::Builder& LveDescriptorPool::Builder::SetMaxSets(U32 count)
	{
		m_maxSets = count;
		return *this;
	}

	UniqueRef<LveDescriptorPool> LveDescriptorPool::Builder::Build() const
	{
		return MakeUniqueRef<LveDescriptorPool>(m_device, m_maxSets, m_poolFlags, m_poolSizes);
	}

	/////////////////////////////////////////////////////////////////////////////////
	// Descriptor pool
	/////////////////////////////////////////////////////////////////////////////////

	LveDescriptorPool::LveDescriptorPool(
		LveDevice& device, U32 maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes)
		: m_device(device)
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<U32>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = poolFlags;

		VkResult result = vkCreateDescriptorPool(m_device.GetDevice(), &descriptorPoolInfo, nullptr, &m_descriptorPool);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create descriptor pool!");
	}

	LveDescriptorPool::~LveDescriptorPool()
	{
		vkDestroyDescriptorPool(m_device.GetDevice(), m_descriptorPool, nullptr);
	}

	bool LveDescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) const
	{
		VkDescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = m_descriptorPool;
		allocateInfo.pSetLayouts = &descriptorSetLayout;
		allocateInfo.descriptorSetCount = 1;

		// Might want to create a "DescriptorPoolManager" class that handles this case and builds a new pool whenever an old pool fills up.
		VkResult result = vkAllocateDescriptorSets(m_device.GetDevice(), &allocateInfo, &descriptorSet);
		return result == VK_SUCCESS;
	}

	void LveDescriptorPool::FreeDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets) const
	{
		vkFreeDescriptorSets(m_device.GetDevice(), m_descriptorPool, static_cast<U32>(descriptorSets.size()), descriptorSets.data());
	}

	void LveDescriptorPool::ResetPool()
	{
		vkResetDescriptorPool(m_device.GetDevice(), m_descriptorPool, 0);
	}

	/////////////////////////////////////////////////////////////////////////////////
	// Descriptor writer
	/////////////////////////////////////////////////////////////////////////////////

	LveDescriptorWriter::LveDescriptorWriter(LveDescriptorSetLayout& descriptorSetLayout, LveDescriptorPool& descriptorPool)
		: m_descriptorSetLayout(descriptorSetLayout), m_descriptorPool(descriptorPool)
	{
	}

	LveDescriptorWriter& LveDescriptorWriter::WriteBuffer(U32 binding, VkDescriptorBufferInfo* bufferInfo)
	{
		ASSERT(m_descriptorSetLayout.m_bindings.count(binding) == 1, "Layout does not contain specified binding!");

		VkDescriptorSetLayoutBinding& bindingDescription = m_descriptorSetLayout.m_bindings[binding];
		ASSERT(bindingDescription.descriptorCount == 1, "Binding single descriptor info, but binding expects multiple!");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = bufferInfo;
		write.descriptorCount = 1;
		m_writes.push_back(write);

		return *this;
	}

	LveDescriptorWriter& LveDescriptorWriter::WriteImage(U32 binding, VkDescriptorImageInfo* imageInfo)
	{
		ASSERT(m_descriptorSetLayout.m_bindings.count(binding) == 1, "Layout does not contain specified binding!");

		VkDescriptorSetLayoutBinding& bindingDescription = m_descriptorSetLayout.m_bindings[binding];
		ASSERT(bindingDescription.descriptorCount == 1, "Binding single descriptor info, but binding expects multiple!");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = imageInfo;
		write.descriptorCount = 1;
		m_writes.push_back(write);

		return *this;
	}

	bool LveDescriptorWriter::Build(VkDescriptorSet& descriptorSet)
	{
		bool success = m_descriptorPool.AllocateDescriptorSet(m_descriptorSetLayout.GetDescriptorSetLayout(), descriptorSet);
		if (!success)
		{
			return false;
		}

		Overwrite(descriptorSet);
		return true;
	}

	void LveDescriptorWriter::Overwrite(VkDescriptorSet& descriptorSet)
	{
		for (VkWriteDescriptorSet& write : m_writes)
		{
			write.dstSet = descriptorSet;
		}

		vkUpdateDescriptorSets(m_descriptorPool.m_device.GetDevice(), m_writes.size(), m_writes.data(), 0, nullptr);
	}

} // namespace lve
