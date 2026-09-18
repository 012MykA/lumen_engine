#pragma once

#include "lumen_engine/core/export.hpp"
#include "lumen_engine/rhi/types.hpp"

#include <memory>

namespace lm::rhi {

struct BufferDesc {
	uint64_t size = 0;
	BufferUsage usage = BufferUsage::VertexBuffer;
	MemoryUsage memory = MemoryUsage::DeviceLocal;
};

struct TextureDesc {
	uint32_t width = 1;
	uint32_t height = 1;
	uint32_t mipLevels = 1;
	Format format = Format::RGBA8_UNORM;
	TextureUsage usage = TextureUsage::Sampled;
	MemoryUsage memory = MemoryUsage::DeviceLocal;
};

class LUMEN_API IDevice {
public:
	virtual ~IDevice() = default;

	virtual BufferHandle CreateBuffer(const BufferDesc &desc, const void *data = nullptr) = 0;
	virtual void DestroyBuffer(BufferHandle handle) = 0;
	// Write host-visible only buffer
	virtual void WriteBuffer(BufferHandle handle, const void *data, uint64_t size, uint64_t offset = 0) = 0;

	virtual TextureHandle CreateTexture(const TextureDesc &desc, const void *data = nullptr) = 0;
	virtual void DestroyTexture(TextureHandle handle) = 0;

	virtual void *GetVkInstance() const = 0;
	virtual void *GetVkDebugUtilsMessengerEXT() const = 0;
	virtual void *GetVkSurfaceKHR() const = 0;
	virtual void *GetVkPhysicalDevice() const = 0;
	virtual void *GetQueueFamilyIndices() const = 0;
	virtual void *GetVkDevice() const = 0;
	virtual void *GetVkCommandPool() const = 0;
	virtual void *GetVkBuffer(BufferHandle handle) const = 0;
	virtual void *GetVkImage(TextureHandle handle) const = 0;
	virtual void *GetVkImageView(TextureHandle handle) const = 0;
};

LUMEN_API std::unique_ptr<IDevice> CreateDevice(GraphicsAPI api, void *window);

LUMEN_API bool IsAvailableAPI(GraphicsAPI api);

} // namespace lm::rhi
