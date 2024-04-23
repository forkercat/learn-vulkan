//
// Created by Junhao Wang (@forkercat) on 4/22/24.
//

#pragma once

#include "core/core.h"

#include "lve_device.h"

namespace lve
{
	// Descriptor set layout
	class LveDescriptorSetLayout
	{
	public:
		class Builder
		{
		public:
			Builder(LveDevice& device)
				: m_device(device)
			{
			}

			Builder& AddBinding(U32 binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, U32 count = 1);
			UniqueRef<LveDescriptorSetLayout> Build() const;

		private:
			LveDevice& m_device;
			std::unordered_map<U32, VkDescriptorSetLayoutBinding> m_bindings{};
		};

	public:
		LveDescriptorSetLayout(LveDevice& device, std::unordered_map<U32, VkDescriptorSetLayoutBinding> bindings);
		~LveDescriptorSetLayout();

		LveDescriptorSetLayout(const LveDescriptorSetLayout&) = delete;
		LveDescriptorSetLayout& operator=(const LveDescriptorSetLayout&) = delete;

		VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

	private:
		LveDevice& m_device;
		VkDescriptorSetLayout m_descriptorSetLayout;
		std::unordered_map<U32, VkDescriptorSetLayoutBinding> m_bindings;

		friend class LveDescriptorWriter;
	};

	// Descriptor pool
	class LveDescriptorPool
	{
	public:
		class Builder
		{
		public:
			Builder(LveDevice& device)
				: m_device(device)
			{
			}

			Builder& AddPoolSize(VkDescriptorType descriptorType, U32 descriptorCount);
			Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
			Builder& SetMaxSets(U32 count);
			UniqueRef<LveDescriptorPool> Build() const;

		private:
			LveDevice& m_device;
			std::vector<VkDescriptorPoolSize> m_poolSizes{};
			U32 m_maxSets = 1000;
			VkDescriptorPoolCreateFlags m_poolFlags = 0;
		};

	public:
		LveDescriptorPool(LveDevice& device, U32 maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes);
		~LveDescriptorPool();

		LveDescriptorPool(const LveDescriptorPool&) = delete;
		LveDescriptorPool& operator=(const LveDescriptorPool&) = delete;

		bool AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) const;
		void FreeDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets) const;
		void ResetPool();

	private:
		LveDevice& m_device;
		VkDescriptorPool m_descriptorPool;

		friend class LveDescriptorWriter;
	};

	// Descriptor writer
	class LveDescriptorWriter
	{
	public:
		LveDescriptorWriter(LveDescriptorSetLayout& descriptorSetLayout, LveDescriptorPool& descriptorPool);
		~LveDescriptorWriter() = default;

		LveDescriptorWriter(const LveDescriptorWriter&) = delete;
		LveDescriptorWriter& operator=(const LveDescriptorWriter&) = delete;

		LveDescriptorWriter& WriteBuffer(U32 binding, VkDescriptorBufferInfo* bufferInfo);
		LveDescriptorWriter& WriteImage(U32 binding, VkDescriptorImageInfo* imageInfo);

		bool Build(VkDescriptorSet& descriptorSet);
		void Overwrite(VkDescriptorSet& descriptorSet);

	private:
		LveDescriptorSetLayout& m_descriptorSetLayout;
		LveDescriptorPool& m_descriptorPool;
		std::vector<VkWriteDescriptorSet> m_writes;
	};

} // namespace lve
