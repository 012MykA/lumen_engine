#pragma once

#include <cstdint>

namespace lm {

class GlfwContext {
public:
	GlfwContext(const GlfwContext &) = delete;
	GlfwContext &operator=(const GlfwContext &) = delete;

	static bool Init();
	static void Shutdown();

	static bool IsInitialized() { return s_RefCount > 0; }

private:
	GlfwContext() = default;
	static inline int32_t s_RefCount{ 0 };
};

} // namespace lm
