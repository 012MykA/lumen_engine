#pragma once

#include "lumen_engine/vulkan_core/device.hpp"
#include "lumen_engine/vulkan_core/instance.hpp"
#include "lumen_engine/vulkan_core/physical_device.hpp"
#include "lumen_engine/vulkan_core/vma_allocator.hpp"
#include "lumen_engine/vulkan_core/volk_loader.hpp"

#include <memory>
#include <string>

namespace lm {

struct GraphicsContextDesc {
	std::string appName = "Lumen Application";
	bool enableValidation = true;
};

class GraphicsContext final {
public:
	explicit GraphicsContext(const GraphicsContextDesc &desc = {});
	~GraphicsContext();

	GraphicsContext(const GraphicsContext &) = delete;
	GraphicsContext &operator=(const GraphicsContext &) = delete;

private:
	lmvk::VolkLoader m_Loader;
	std::unique_ptr<lmvk::Instance> m_Instance;
	std::unique_ptr<lmvk::PhysicalDevice> m_PhysicalDevice;
	std::unique_ptr<lmvk::Device> m_Device;
	std::unique_ptr<lmvk::VmaAllocatorWrapper> m_Allocator;
};

} // namespace lm
