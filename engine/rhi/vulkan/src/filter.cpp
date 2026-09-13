#include "lumen_engine/io/log.hpp"
#include "lumen_engine/rhi/vulkan/vulkan_device.hpp"
#include "vulkan_validation.hpp"

#include <algorithm>
#include <format>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace lm::rhi {

namespace {

template <typename AvailableRange, typename NameFn>
std::vector<const char *> FilterSupportedNames(const AvailableRange &available, NameFn getName,
		const std::vector<const char *> &wanted, bool required, std::string_view kind) {
	std::vector<const char *> enabled;
	for (const char *name : wanted) {
		bool found = std::ranges::any_of(available, [&](const auto &item) {
			return std::strcmp(getName(item), name) == 0;
		});
		if (found) {
			enabled.push_back(name);
		} else if (required) {
			throw std::runtime_error(std::format("Required Vulkan {} not supported: {}", kind, name));
		} else {
			LUMEN_CORE_WARN("Optional Vulkan {} not supported: {}", kind, name);
		}
	}
	return enabled;
}

} // namespace

std::vector<const char *> VulkanDevice::FilterSupportedDeviceExtensions(
		VkPhysicalDevice device, const std::vector<const char *> &wanted, bool required) const {
	return FilterSupportedNames(EnumerateDeviceExtensions(device), [](const VkExtensionProperties &p) { return p.extensionName; }, wanted, required, "device extension");
}

std::vector<const char *> VulkanDevice::FilterSupportedInstanceLayers(
		const std::vector<const char *> &wanted, bool required) const {
	return FilterSupportedNames(EnumerateInstanceLayers(), [](const VkLayerProperties &p) { return p.layerName; }, wanted, required, "layer");
}

std::vector<const char *> VulkanDevice::FilterSupportedInstanceExtensions(
		const std::vector<const char *> &wanted, bool required) const {
	return FilterSupportedNames(EnumerateInstanceExtensions(), [](const VkExtensionProperties &p) { return p.extensionName; }, wanted, required, "instance extension");
}

} // namespace lm::rhi
