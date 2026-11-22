#include "lumen_engine/gfx/vulkan/vk_descriptors.hpp"

#include <algorithm>

namespace lm {

// --- Layout builder ---

DescriptorSetLayoutBuilder &lm::DescriptorSetLayoutBuilder::AddBinding(
		uint32_t binding,
		VkDescriptorType type,
		VkShaderStageFlags stages,
		uint32_t count) {
	m_Bindings.push_back(VkDescriptorSetLayoutBinding{
			.binding = binding,
			.descriptorType = type,
			.descriptorCount = count,
			.stageFlags = stages,
	});
	return *this;
}

VkDescriptorSetLayout DescriptorSetLayoutBuilder::Build(VkDevice device) const {
	VkDescriptorSetLayoutCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(m_Bindings.size()),
		.pBindings = m_Bindings.data(),
	};
	VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
	CHECK_VK(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layout));
	return layout;
}

// --- Allocator ---

DescriptorAllocator::DescriptorAllocator(
		VkDevice device,
		uint32_t initialSets,
		std::span<const PoolRatio> ratios) :
		m_Device{ device },
		m_Ratios{ ratios.begin(), ratios.end() },
		m_SetsPerPool{ initialSets } {}

DescriptorAllocator::~DescriptorAllocator() {
	for (auto p : m_Ready) {
		vkDestroyDescriptorPool(m_Device, p, nullptr);
	}
	for (auto p : m_Full) {
		vkDestroyDescriptorPool(m_Device, p, nullptr);
	}
}

VkDescriptorSet DescriptorAllocator::Allocate(VkDescriptorSetLayout layout) {
	VkDescriptorPool pool = AcquirePool();

	VkDescriptorSetAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &layout,
	};
	VkDescriptorSet set{ VK_NULL_HANDLE };
	VkResult res = vkAllocateDescriptorSets(m_Device, &allocInfo, &set);

	if (res == VK_ERROR_OUT_OF_POOL_MEMORY || res == VK_ERROR_FRAGMENTED_POOL) {
		m_Full.push_back(pool);
		pool = AcquirePool();
		allocInfo.descriptorPool = pool;
		res = vkAllocateDescriptorSets(m_Device, &allocInfo, &set);
	}
	CHECK_VK(res);

	m_Ready.push_back(pool);
	return set;
}

void DescriptorAllocator::ResetPools() {
	for (auto p : m_Full) {
		CHECK_VK(vkResetDescriptorPool(m_Device, p, 0));
		m_Ready.push_back(p);
	}
	m_Full.clear();
	for (auto p : m_Ready) {
		CHECK_VK(vkResetDescriptorPool(m_Device, p, 0));
	}
}

VkDescriptorPool DescriptorAllocator::CreatePool(uint32_t sets) const {
	std::vector<VkDescriptorPoolSize> sizes;
	for (const auto &r : m_Ratios) {
		VkDescriptorPoolSize size{
			.type = r.type,
			.descriptorCount = std::max(1u, static_cast<uint32_t>(r.perSet * sets)),
		};
		sizes.push_back(size);
	}

	VkDescriptorPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = sets,
		.poolSizeCount = static_cast<uint32_t>(sizes.size()),
		.pPoolSizes = sizes.data(),
	};
	VkDescriptorPool pool{ VK_NULL_HANDLE };
	CHECK_VK(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &pool));
	return pool;
}

VkDescriptorPool DescriptorAllocator::AcquirePool() {
	if (!m_Ready.empty()) {
		VkDescriptorPool pool = m_Ready.back();
		m_Ready.pop_back();
		return pool;
	}
	VkDescriptorPool pool = CreatePool(m_SetsPerPool);
	m_SetsPerPool = std::min(static_cast<uint32_t>(m_SetsPerPool * 1.5), 4096u);
	return pool;
}

// --- Writer ---

DescriptorWriter &DescriptorWriter::WriteBuffer(
		uint32_t binding,
		VkBuffer buffer,
		VkDeviceSize size,
		VkDeviceSize offset,
		VkDescriptorType type) {
	auto &info = m_Buffers.emplace_back(VkDescriptorBufferInfo{ buffer, offset, size });

	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = type,
		.pBufferInfo = &info,
	};
	m_Writes.push_back(write);
	return *this;
}

DescriptorWriter &DescriptorWriter::WriteImage(
		uint32_t binding,
		VkImageView view,
		VkSampler sampler,
		VkImageLayout layout,
		VkDescriptorType type) {
	auto &info = m_Images.emplace_back(VkDescriptorImageInfo{ sampler, view, layout });

	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = type,
		.pImageInfo = &info,
	};
	m_Writes.push_back(write);
	return *this;
}

void DescriptorWriter::Update(VkDevice device, VkDescriptorSet set) {
	for (auto &w : m_Writes) {
		w.dstSet = set;
	}
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(m_Writes.size()), m_Writes.data(), 0, nullptr);
}

} // namespace lm
