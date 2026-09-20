#pragma once

#include "lumen_engine/core/export.hpp"
#include "lumen_engine/rhi/types.hpp"

#include <memory>
#include <span>
#include <vector>

namespace lm::rhi {

struct BufferDesc {
	uint64_t size = 0;
	BufferUsage usage = BufferUsage::VertexBuffer;
	MemoryUsage memory = MemoryUsage::DeviceLocal;
};

struct TextureDesc {
	static constexpr uint32_t k_AutoCalcMipLevels = 0;

	uint32_t width = 1;
	uint32_t height = 1;
	uint32_t mipLevels = 1;
	bool generateMipmaps = false;
	Format format = Format::RGBA8_UNORM;
	TextureUsage usage = TextureUsage::Sampled;
	MemoryUsage memory = MemoryUsage::DeviceLocal;
};

struct SamplerDesc {
	static constexpr float k_AutoAnisotropy = 0.0f;

	Filter magFilter = Filter::Linear;
	Filter minFilter = Filter::Linear;
	AddressMode addressModeU = AddressMode::Repeat;
	AddressMode addressModeV = AddressMode::Repeat;
	AddressMode addressModeW = AddressMode::Repeat;
	bool anisotropyEnabled = true;
	float maxAnisotropy = k_AutoAnisotropy;
};

struct GraphicsPipelineDesc {
	std::vector<PipelineShaderStageDesc> shaderStages;

	std::vector<VertexBinding> vertexBindings;

	PrimitiveTopology topology = PrimitiveTopology::TriangleList;
	CullMode cullMode = CullMode::Back;
	FrontFace frontFace = FrontFace::CounterClockwise;

	DepthStencilState depthStencil{};
	ColorBlendState colorBlend{};

	std::vector<Format> colorAttachmentFormats;
	Format depthAttachmentFormat = Format::Undefined;

	void *pipelineLayout;
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

	virtual SamplerHandle CreateSampler(const SamplerDesc &desc) = 0;
	virtual void DestroySampler(SamplerHandle handle) = 0;

	virtual ShaderHandle CreateShader(std::span<const uint32_t> spirvCode) = 0;
	virtual void DestroyShader(ShaderHandle handle) = 0;

	virtual PipelineHandle CreatePipeline(const GraphicsPipelineDesc &desc) = 0;
	virtual void DestroyPipeline(PipelineHandle handle) = 0;

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
	virtual void *GetVkSampler(SamplerHandle handle) const = 0;
	virtual void *GetVkShaderModule(ShaderHandle handle) const = 0;
	virtual void *GetVkPipeline(PipelineHandle handle) const = 0;
};

LUMEN_API std::unique_ptr<IDevice> CreateDevice(GraphicsAPI api, void *window);

LUMEN_API bool IsAvailableAPI(GraphicsAPI api);

} // namespace lm::rhi
