#pragma once

namespace lm {

class GlfwContext {
public:
	GlfwContext(const GlfwContext &) = delete;
	GlfwContext &operator=(const GlfwContext &) = delete;

	static bool Init();
	static void Shutdown();

	static bool IsInitialized() { return s_Initialized; }

private:
	GlfwContext() = default;
	static inline bool s_Initialized{ false };
};

} // namespace lm
