#pragma once

#include <volk.h>

#include <string>
#include <vector>

namespace lm::vk {

struct InstanceDesc {
	std::string appName = "Lumen Application";
	bool enableValidation = true;
	std::vector<const char *> extraExtensions;
};

class Instance final {
public:
	explicit Instance(const InstanceDesc &desc);
	~Instance();

	Instance(const Instance &) = delete;
	Instance &operator=(const Instance &) = delete;
	Instance(Instance &&other) noexcept;
	Instance &operator=(Instance &&other) noexcept;

	VkInstance Handle() const { return m_Instance; }
	bool ValidationEnabled() const { return m_DebugMessenger != VK_NULL_HANDLE; }

private:
	std::vector<const char *> FilterSupportedInstanceLayers(const std::vector<const char *> &wanted, bool required) const;
	std::vector<const char *> FilterSupportedInstanceExtensions(const std::vector<const char *> &wanted, bool required) const;

	void CreateVkInstance(const InstanceDesc &desc);
	void SetupDebugMessenger();
	void DestroyDebugMessenger();

private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
};

} //namespace lm::vk
