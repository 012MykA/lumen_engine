#include "lumen_engine/gfx/vulkan/vk_debug.hpp"

#include <string>

namespace lm::debug {

namespace {
bool s_Enabled = false;
} // namespace

void SetEnabled(bool enabled) {
	s_Enabled = enabled;
}

bool IsEnabled() {
	return s_Enabled;
}

void SetObjectName(VkDevice device, VkObjectType type, uint64_t handle, std::string_view name) {
	if (!s_Enabled || !vkSetDebugUtilsObjectNameEXT || handle == 0) {
		return;
	}
	const std::string str{ name };
	VkDebugUtilsObjectNameInfoEXT info{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
		.objectType = type,
		.objectHandle = handle,
		.pObjectName = str.c_str(),
	};
	CHECK_VK(vkSetDebugUtilsObjectNameEXT(device, &info));
}

void BeginLabel(VkCommandBuffer cmd, const char *name, const Color &color) {
	if (!s_Enabled || !vkCmdBeginDebugUtilsLabelEXT) {
		return;
	}
	VkDebugUtilsLabelEXT label{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		.pLabelName = name,
		.color = { color.r, color.g, color.b, color.a },
	};
	vkCmdBeginDebugUtilsLabelEXT(cmd, &label);
}

void EndLabel(VkCommandBuffer cmd) {
	if (!s_Enabled || !vkCmdEndDebugUtilsLabelEXT) {
		return;
	}
	vkCmdEndDebugUtilsLabelEXT(cmd);
}

ScopedLabel::ScopedLabel(VkCommandBuffer cmd, const char *name, const Color &color) :
		m_Cmd{ cmd } {
	BeginLabel(m_Cmd, name, color);
}

ScopedLabel::~ScopedLabel() {
	EndLabel(m_Cmd);
}

} // namespace lm::debug
