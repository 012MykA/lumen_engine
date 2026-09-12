#pragma once

#include "lumen_engine/core/export.hpp"
#include "lumen_engine/rhi/types.hpp"

#include <memory>

namespace lm::rhi {

class LUMEN_API IDevice {
public:
	virtual ~IDevice() = default;

	[[nodiscard]] virtual GraphicsAPI GetAPI() const = 0;
};

LUMEN_API std::unique_ptr<IDevice> CreateDevice(GraphicsAPI api, void *window);

LUMEN_API bool IsAvailableAPI(GraphicsAPI api);

} // namespace lm::rhi
