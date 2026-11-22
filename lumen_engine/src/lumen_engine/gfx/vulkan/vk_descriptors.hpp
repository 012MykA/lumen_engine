#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

#include <deque>
#include <span>
#include <vector>

namespace lm {

class DescriptorSetLayoutBuilder final {
public:
	DescriptorSetLayoutBuilder &AddBinding(
			uint32_t binding,
			VkDescriptorType type,
			VkShaderStageFlags stages,
			uint32_t count = 1);

	VkDescriptorSetLayout Build(VkDevice device) const;

private:
	std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
};

class DescriptorAllocator final {
public:
	struct PoolRatio {
		VkDescriptorType type;
		float perSet;
	};

	DescriptorAllocator(
			VkDevice device,
			uint32_t initialSets,
			std::span<const PoolRatio> ratios);
	~DescriptorAllocator();

	DescriptorAllocator(const DescriptorAllocator &) = delete;
	DescriptorAllocator &operator=(const DescriptorAllocator &) = delete;

	VkDescriptorSet Allocate(VkDescriptorSetLayout layout);
	void ResetPools();

private:
	VkDescriptorPool CreatePool(uint32_t sets) const;
	VkDescriptorPool AcquirePool();

private:
	VkDevice m_Device{ VK_NULL_HANDLE };
	std::vector<PoolRatio> m_Ratios;
	std::vector<VkDescriptorPool> m_Ready, m_Full;
	uint32_t m_SetsPerPool;
};

class DescriptorWriter final {
public:
	DescriptorWriter &WriteBuffer(
			uint32_t binding,
			VkBuffer buffer,
			VkDeviceSize size,
			VkDeviceSize offset,
			VkDescriptorType type);

	DescriptorWriter &WriteImage(
			uint32_t binding,
			VkImageView view,
			VkSampler sampler,
			VkImageLayout layout,
			VkDescriptorType type);

	void Update(VkDevice device, VkDescriptorSet set);

private:
	std::deque<VkDescriptorBufferInfo> m_Buffers;
	std::deque<VkDescriptorImageInfo> m_Images;
	std::vector<VkWriteDescriptorSet> m_Writes;
};

} // namespace lm
