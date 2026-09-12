#pragma once

#include "lumen_engine/core/export.hpp"
#include "lumen_engine/rhi/types.hpp"

#include <memory>

namespace lm::rhi {

class LUMEN_API IDevice {
public:
	virtual ~IDevice() = default;

	virtual GraphicsAPI GetAPI() const = 0;

	virtual void *GetVkInstance() const = 0;
	virtual void *GetVkDebugUtilsMessengerEXT() const = 0;
	virtual void *GetVkSurfaceKHR() const = 0;
};

LUMEN_API std::unique_ptr<IDevice> CreateDevice(GraphicsAPI api, void *window);

LUMEN_API bool IsAvailableAPI(GraphicsAPI api);

} // namespace lm::rhi
