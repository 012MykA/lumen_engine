#include "lumen_engine/vulkan_core/volk_loader.hpp"

#include "lumen_engine/vulkan_core/check_vk.hpp"

#include <volk.h>

namespace lmvk {

VolkLoader::VolkLoader() {
	CHECK_VK(volkInitialize());
}

VolkLoader::~VolkLoader() {
	volkFinalize();
}

} // namespace lmvk
