#include "lumen_engine/gfx/renderer.hpp"

#include "lumen_engine/gfx/vulkan/vk_shader_module.hpp"
#include "lumen_engine/gfx/vulkan/vk_utils.hpp"

#include <climits>

namespace lm {

namespace {

static constexpr auto k_PipelineCachePath = "assets/cache/vk_pipeline_cache.bin";

}

Renderer::Renderer(const Window &window) :
		m_Window{ window },
		m_Context{ m_Window },
		m_Swapchain{ m_Context },
		m_PipelineCache{ m_Context.GetVkDevice(), k_PipelineCachePath } {
	CreateFrames();
	CreatePipelines();
}

Renderer::~Renderer() {
	m_Context.DeviceWaitIdle();
	DestroyFrames();
}

void Renderer::OnRender() {
	VkDevice device = m_Context.GetVkDevice();
	if (m_NeedsResize) {
		if (!m_Swapchain.Recreate()) {
			return;
		}
		m_NeedsResize = false;
	}

	FrameData &frame = m_Frames[m_FrameIndex];
	CHECK_VK(vkWaitForFences(device, 1, &frame.inFlight, VK_TRUE, UINT64_MAX));

	uint32_t imageIndex = 0;
	VkResult res = vkAcquireNextImageKHR(device, m_Swapchain.GetVkSwapchain(), UINT64_MAX,
			frame.imageAvailable, VK_NULL_HANDLE, &imageIndex);
	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		m_NeedsResize = true;
		return;
	}
	if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
		CHECK_VK(res);
	}
	CHECK_VK(vkResetFences(device, 1, &frame.inFlight));

	CHECK_VK(vkResetCommandPool(device, frame.pool, 0));
	VkCommandBufferBeginInfo begin{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	CHECK_VK(vkBeginCommandBuffer(frame.cmd, &begin));
	{
		RecordCommands(frame.cmd, imageIndex);
	}
	CHECK_VK(vkEndCommandBuffer(frame.cmd));

	VkSemaphore renderFinished = m_Swapchain.GetRenderFinished(imageIndex);
	VkSemaphoreSubmitInfo wait{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = frame.imageAvailable,
		.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
	};
	VkSemaphoreSubmitInfo signal{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = renderFinished,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
	};
	VkCommandBufferSubmitInfo cmdInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = frame.cmd,
	};
	VkSubmitInfo2 submit{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &wait,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmdInfo,
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signal,
	};
	CHECK_VK(vkQueueSubmit2(m_Context.GetGraphicsQueue(), 1, &submit, frame.inFlight));

	VkSwapchainKHR swapchain = m_Swapchain.GetVkSwapchain();
	VkPresentInfoKHR present{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &renderFinished,
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &imageIndex,
	};
	res = vkQueuePresentKHR(m_Context.GetPresentQueue(), &present);
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
		m_NeedsResize = true;
	} else {
		CHECK_VK(res);
	}

	m_FrameIndex = (m_FrameIndex + 1) % k_FramesInFlight;
}

void Renderer::OnResize(WindowExtent extent) {
	m_NeedsResize = true;
}

void Renderer::RecordCommands(VkCommandBuffer cmd, uint32_t imageIndex) {
	VkImage image = m_Swapchain.GetImage(imageIndex);

	TransitionImage(cmd, image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

	VkRenderingAttachmentInfo color{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = m_Swapchain.GetImageView(imageIndex),
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue{ .color{ { 0.02f, 0.03f, 0.08f, 1.0f } } },
	};
	VkRenderingInfo rendering{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea{ { 0, 0 }, m_Swapchain.GetExtent() },
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color,
	};
	vkCmdBeginRendering(cmd, &rendering);
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_TrianglePipeline->GetVkPipeline());
		const VkExtent2D extent = m_Swapchain.GetExtent();
		VkViewport viewport{
			0.0f, 0.0f,
			static_cast<float>(extent.width), static_cast<float>(extent.height),
			0.0f, 1.0f
		};
		VkRect2D scissor{ { 0, 0 }, extent };
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		vkCmdDraw(cmd, 3, 1, 0, 0);
	}
	vkCmdEndRendering(cmd);

	TransitionImage(cmd, image,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_2_NONE, 0);
}

void Renderer::CreateFrames() {
	VkDevice device = m_Context.GetVkDevice();
	for (auto &f : m_Frames) {
		VkCommandPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = m_Context.GetQueueFamilyIndices().graphicsFamily.value(),
		};
		CHECK_VK(vkCreateCommandPool(device, &poolInfo, nullptr, &f.pool));

		VkCommandBufferAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = f.pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		CHECK_VK(vkAllocateCommandBuffers(device, &allocInfo, &f.cmd));

		VkFenceCreateInfo fenceInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};
		CHECK_VK(vkCreateFence(device, &fenceInfo, nullptr, &f.inFlight));

		VkSemaphoreCreateInfo semInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		CHECK_VK(vkCreateSemaphore(device, &semInfo, nullptr, &f.imageAvailable));
	}
}

void Renderer::CreatePipelines() {
	VkDevice device = m_Context.GetVkDevice();

	const auto shaderCode = ReadSpirV("assets/shaders/bin/triangle.spv");
	VulkanShaderModule shader{ device, shaderCode };
	m_TrianglePipeline.emplace(GraphicsPipelineBuilder{}
					.SetShader(shader.GetVkShaderModule())
					.SetColorFormat(m_Swapchain.GetFormat())
					.Build(device, m_PipelineCache.GetVkPipelineCache()));
}

void Renderer::DestroyFrames() {
	VkDevice device = m_Context.GetVkDevice();
	for (auto &f : m_Frames) {
		vkDestroySemaphore(device, f.imageAvailable, nullptr);
		vkDestroyFence(device, f.inFlight, nullptr);
		vkDestroyCommandPool(device, f.pool, nullptr);
	}
}

} // namespace lm
