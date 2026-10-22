#pragma once

#include "lumen_engine/vulkan_core/device.hpp"
#include "lumen_engine/vulkan_core/instance.hpp"
#include "lumen_engine/vulkan_core/physical_device.hpp"
#include "lumen_engine/vulkan_core/vma_allocator.hpp"
#include "lumen_engine/vulkan_core/volk_loader.hpp"

#include <string>

namespace lm {

struct GraphicsContextDesc {
	std::string appName = "Lumen Application";
	bool enableValidation = true;
};

class Window;

class GraphicsContext final {
public:
	explicit GraphicsContext(const Window &target, const GraphicsContextDesc &desc);
	~GraphicsContext();

	GraphicsContext(const GraphicsContext &) = delete;
	GraphicsContext &operator=(const GraphicsContext &) = delete;

private:
	lmvk::VolkLoader m_Loader;
	lmvk::Instance m_Instance;
	lmvk::PhysicalDevice m_PhysicalDevice;
	lmvk::Device m_Device;
	lmvk::VmaAllocatorWrapper m_Allocator;
};

} // namespace lm
