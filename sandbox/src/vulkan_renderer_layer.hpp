#pragma once

#include <lumen_engine/lumen_engine.hpp>
#include <vulkan/vulkan.h>

class VulkanRendererLayer : public lm::Layer {
public:
	explicit VulkanRendererLayer(lm::Window *window);

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(lm::Event &e) override;
	virtual void OnRender() override;

private:
	lm::Window *m_Window;
	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
};
