#pragma once

#include <cstdint>

namespace lm {

class WindowContext {
public:
	WindowContext();
	~WindowContext();

	bool Init();
	void Shutdown();

	bool IsInitialized();

private:
	inline static int32_t s_RefCount{ 0 };
};

} // namespace lm
