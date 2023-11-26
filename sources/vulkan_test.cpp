#pragma once
#include "../headers/vulkan_context.h"

int main() {
	Vk::Instance instance;
	instance.init_extensions();
	instance.init_layers();
	instance.create();

	Vk::Device device(instance);
	Vk::CommandPool(device, Vk::QueueFamily::GRAPHICS) command_pool;
	Vk::CommandBuffer command_buffer(device, Vk::QueueFamily::GRAPHICS, command_pool);
	Vk::Swapchain swapchain(device);
	Vk::RenderPass renderpass(device, swapchain.format, Vk::QueueFamily::GRAPHICS);
	Vk::GraphicsPipeline pipeline(device, renderpass, swapchain);
	return 0;
}