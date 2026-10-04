#pragma once

#include "lumen_engine/core/export.hpp"
#include "lumen_engine/vulkan_core/physical_device.hpp"

#include <volk.h>

namespace lm::vk {

class LUMEN_API Device final {
public:
	Device(const PhysicalDevice &physicalDevice);
	~Device();

	Device(const Device &) = delete;
	Device &operator=(const Device &) = delete;
	Device(Device &&other) noexcept;
	Device &operator=(Device &&other) noexcept;

	VkDevice Handle() const { return m_Device; }
	VkQueue GraphicsQueue() const { return m_GraphicsQueue; }
	VkQueue PresentQueue() const { return m_PresentQueue; }
	VkQueue ComputeQueue() const { return m_ComputeQueue; }

	void WaitIdle() const;

private:
	VkDevice m_Device = VK_NULL_HANDLE;
	VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
	VkQueue m_PresentQueue = VK_NULL_HANDLE;
	VkQueue m_ComputeQueue = VK_NULL_HANDLE;
};

} // namespace lm::vk
