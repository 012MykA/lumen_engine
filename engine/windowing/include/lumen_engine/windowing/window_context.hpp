#pragma once

#include <cstdint>

namespace lm {

class WindowContext {
public:
	WindowContext(const WindowContext &) = delete;
	WindowContext &operator=(const WindowContext &) = delete;

	static bool Init();
	static void Shutdown();

	static bool IsInitialized();

private:
	WindowContext() = default;
	inline static int32_t s_RefCount{ 0 };
};

} // namespace lm
