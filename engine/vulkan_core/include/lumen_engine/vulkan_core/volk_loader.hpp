#pragma once

namespace lmvk {

class VolkLoader final {
public:
	VolkLoader();
	~VolkLoader();

	VolkLoader(const VolkLoader &) = delete;
	VolkLoader &operator=(const VolkLoader &) = delete;
};

} // namespace lmvk
