#include "lumen_engine/gfx/renderer.hpp"

#include "lumen_engine/gfx/vulkan/vk_shader_module.hpp"
#include "lumen_engine/gfx/vulkan/vk_utils.hpp"
#include "lumen_engine/gfx/vulkan/vk_vertex.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <format>

#define PIPELINE_CACHE_PATH "assets/cache/vk_pipeline_cache.bin"
#define SHADER_PATH "assets/shaders/bin/shader.spv"
#define TEXTURE_PATH "assets/textures/texture4k.jpg"

namespace lm {

namespace {

std::pair<std::vector<Vertex>, std::vector<uint32_t>> BuildScene() {
	std::vector<Vertex> vertices = {
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
	};

	std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

	return { std::move(vertices), std::move(indices) };
}

} //namespace

Renderer::Renderer(const Window &window) :
		m_Window{ window },
		m_Context{ m_Window },
		m_Swapchain{ m_Context },
		m_PipelineCache{ m_Context.GetVkDevice(), PIPELINE_CACHE_PATH },
		m_Uploader{
			m_Context.GetVkDevice(),
			m_Context.GetGraphicsQueue(),
			m_Context.GetQueueFamilyIndices().graphicsFamily.value()
		} {
	CreateFrames();

	CreateTexture();
	CreateDescriptors();

	CreatePipelines();
	CreateDepthImage();

	auto [vertices, indices] = BuildScene();
	m_IndexCount = static_cast<uint32_t>(indices.size());

	m_VertexBuffer = m_Uploader.UploadBuffer(vertices.data(), vertices.size() * sizeof(Vertex),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			m_Context.GetAllocator());

	m_IndexBuffer = m_Uploader.UploadBuffer(indices.data(), indices.size() * sizeof(uint32_t),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			m_Context.GetAllocator());
}

Renderer::~Renderer() {
	m_Context.DeviceWaitIdle();
	DestroyFrames();

	VkDevice device = m_Context.GetVkDevice();
	vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, m_SetLayout, nullptr);
}

void Renderer::OnRender() {
	VkDevice device = m_Context.GetVkDevice();
	if (m_NeedsResize) {
		if (!m_Swapchain.Recreate()) {
			return;
		}
		CreateDepthImage();
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

void Renderer::OnResize() {
	m_NeedsResize = true;
}

void Renderer::RecordCommands(VkCommandBuffer cmd, uint32_t imageIndex) {
	VkImage image = m_Swapchain.GetImage(imageIndex);

	TransitionImage(cmd, image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

	TransitionImage(cmd, m_DepthImage.GetImage(),
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, 0,
			VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT);

	VkRenderingAttachmentInfo color{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = m_Swapchain.GetImageView(imageIndex),
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue{ .color{ { 0.02f, 0.03f, 0.08f, 1.0f } } },
	};
	VkRenderingAttachmentInfo depth{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = m_DepthImage.GetImageView(),
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.clearValue{ .depthStencil{ 1.0f, 0 } },
	};
	VkRenderingInfo rendering{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea{ { 0, 0 }, m_Swapchain.GetExtent() },
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color,
		.pDepthAttachment = &depth,
	};
	vkCmdBeginRendering(cmd, &rendering);
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetVkPipeline());

		const VkExtent2D extent = m_Swapchain.GetExtent();
		VkViewport viewport{
			0.0f, 0.0f,
			static_cast<float>(extent.width), static_cast<float>(extent.height),
			0.0f, 1.0f
		};
		VkRect2D scissor{ { 0, 0 }, extent };
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		VkBuffer vb = m_VertexBuffer.GetVkBuffer();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(cmd, 0, 1, &vb, &offset);
		vkCmdBindIndexBuffer(cmd, m_IndexBuffer.GetVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdPushConstants(cmd, m_Pipeline->GetVkPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &m_ViewProj);

		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_Pipeline->GetVkPipelineLayout(), 0, 1, &m_TextureSet, 0, nullptr);

		vkCmdDrawIndexed(cmd, m_IndexCount, 1, 0, 0, 0);
	}
	vkCmdEndRendering(cmd);

	TransitionImage(cmd, image,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_2_NONE, 0,
			VK_IMAGE_ASPECT_COLOR_BIT);
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

	constexpr std::array bindings = { Vertex::GetBindingDescription() };
	constexpr std::array attributes = Vertex::GetAttributeDescriptions();

	const auto shaderCode = ReadSpirV(SHADER_PATH);
	VulkanShaderModule shader{ device, shaderCode };

	m_Pipeline.emplace(GraphicsPipelineBuilder{}
					.SetShader(shader.GetVkShaderModule())
					.SetVertexInput(bindings, attributes)
					.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
					.SetColorFormat(m_Swapchain.GetFormat())
					.SetDepthFormat(k_DepthFormat)
					.AddDescriptorSetLayout(m_SetLayout)
					.SetPushConstants<glm::mat4>(/*stage=*/VK_SHADER_STAGE_VERTEX_BIT)
					.Build(device, m_PipelineCache.GetVkPipelineCache()));
}

void Renderer::CreateDepthImage() {
	m_DepthImage = VulkanImage{
		m_Context.GetVkDevice(),
		m_Context.GetAllocator(),
		m_Swapchain.GetExtent(),
		k_DepthFormat,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
	};
}

void Renderer::CreateDescriptors() {
	VkDevice device = m_Context.GetVkDevice();

	VkDescriptorSetLayoutBinding binding{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};
	VkDescriptorSetLayoutCreateInfo layoutInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &binding,
	};
	CHECK_VK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_SetLayout));

	VkDescriptorPoolSize poolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 };
	VkDescriptorPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = 1,
		.poolSizeCount = 1,
		.pPoolSizes = &poolSize,
	};
	CHECK_VK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool));

	VkDescriptorSetAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = m_DescriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &m_SetLayout,
	};
	CHECK_VK(vkAllocateDescriptorSets(device, &allocInfo, &m_TextureSet));

	VkDescriptorImageInfo imageInfo{
		.sampler = m_Sampler->GetVkSampler(),
		.imageView = m_Texture.GetImageView(),
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_TextureSet,
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = &imageInfo,
	};
	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void Renderer::CreateTexture() {
	int texWidth, texHeight, texChannels;
	stbi_uc *pixels = stbi_load(TEXTURE_PATH, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error{ std::format("failed to load texture: {}", TEXTURE_PATH) };
	}
	m_Texture = m_Uploader.UploadImage(pixels, texWidth, texHeight,
			VK_FORMAT_R8G8B8A8_SRGB, m_Context.GetAllocator());

	stbi_image_free(pixels);

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(m_Context.GetVkPhysicalDevice(), &props);
	m_Sampler.emplace(m_Context.GetVkDevice(), props.limits.maxSamplerAnisotropy);
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
