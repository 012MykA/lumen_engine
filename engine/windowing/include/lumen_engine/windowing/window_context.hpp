#pragma once

#include <cstdint>

namespace lm {

class WindowContext {
public:
	WindowContext();
	~WindowContext();

	WindowContext(const WindowContext &) = delete;
	WindowContext &operator=(const WindowContext &) = delete;

	static bool IsInitialized();

private:
	bool Init();
	void Shutdown();

private:
	inline static int32_t s_RefCount{ 0 };
};

} // namespace lm
