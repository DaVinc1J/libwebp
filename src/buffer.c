#include "headers/buffer.h"
#include "headers/validation.h"

void create_buffer(_app *p_app, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, VkBuffer *p_buffer, VmaAllocation *p_allocation) {
	VkBufferCreateInfo buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	VmaAllocationCreateInfo alloc_info = {
		.usage = memory_usage,
	};

	if (vmaCreateBuffer(p_app->mem.alloc, &buffer_create_info, &alloc_info, p_buffer, p_allocation, NULL) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"vma => failed to create buffer"
		);
		exit(EXIT_FAILURE);
	}
}

void create_command_pool(_app *p_app) {
	VkCommandPoolCreateInfo command_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = p_app->device.queue_indices.graphics_family,
	};

	if (vkCreateCommandPool(p_app->device.logical, &command_pool_create_info, NULL, &p_app->cmd.pool) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"command pool => failed to create command pool"
		);
		exit(EXIT_FAILURE);
	}
}

void create_command_buffers(_app *p_app) {
	p_app->cmd.buffers = malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo command_buffer_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = p_app->cmd.pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT,
	};

	if (vkAllocateCommandBuffers(p_app->device.logical, &command_buffer_alloc_info, p_app->cmd.buffers) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"command buffer => failed to allocate command buffer"
		);
		exit(EXIT_FAILURE);
	}
}

void record_command_buffer(_app *p_app, VkCommandBuffer command_buffer, uint32_t image_index) {
	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "command buffer => failed to begin record");
		exit(EXIT_FAILURE);
	}

	VkClearValue clear_value = { .color = {{0.0f, 0.0f, 0.0f, 0.0f}} };

	VkRenderPassBeginInfo render_pass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = p_app->pipeline.render_pass,
		.framebuffer = p_app->pipeline.swapchain_framebuffers[image_index],
		.renderArea = { .offset = {0, 0}, .extent = p_app->swp.extent },
		.clearValueCount = 1,
		.pClearValues = &clear_value,
	};

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = {
		.x = 0.0f, .y = 0.0f,
		.width = (float)p_app->swp.extent.width,
		.height = (float)p_app->swp.extent.height,
		.minDepth = 0.0f, .maxDepth = 1.0f,
	};
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor = { .offset = {0, 0}, .extent = p_app->swp.extent };
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	if (p_app->image.created) {
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_app->pipeline.pipeline);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
													 p_app->pipeline.layout, 0, 1,
													 &p_app->descriptor.sets[p_app->sync.frame_index], 0, NULL);
		_pc pc = {0};
		float inner_w = (float)p_app->swp.extent.width  - 2.0f * p_app->config.border_px;
		float inner_h = (float)p_app->swp.extent.height - p_app->config.title_px - p_app->config.border_px;
		float img = (float)p_app->output.width / p_app->output.height;
		float win = inner_w / inner_h;
		pc.uv_scale[0] = 1.0f;
		pc.uv_scale[1] = 1.0f;
		if (win > img) {
			pc.uv_scale[0] = img / win;
		} else {
			pc.uv_scale[1] = win / img;
		}

		pc.border_uv[0] = p_app->config.border_px / (float)p_app->swp.extent.width;
		pc.border_uv[1] = p_app->config.border_px / (float)p_app->swp.extent.height;

		pc.title_uv = p_app->config.title_px / (float)p_app->swp.extent.height;

		glm_vec4_copy(p_app->config.title_colour, pc.title_colour);
		glm_vec4_copy(p_app->config.border_colour, pc.border_colour);
		glm_vec4_copy(p_app->config.background_colour, pc.background_colour);

		vkCmdPushConstants(command_buffer, p_app->pipeline.layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(_pc), &pc);

		vkCmdDraw(command_buffer, 3, 1, 0, 0);
	}

	vkCmdEndRenderPass(command_buffer);

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "command buffer => failed to end record");
		exit(EXIT_FAILURE);
	}
}

VkCommandBuffer begin_single_time_commands(_app *p_app) {
	VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandPool = p_app->cmd.pool,
		.commandBufferCount = 1,
	};

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(p_app->device.logical, &alloc_info, &command_buffer);

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	vkBeginCommandBuffer(command_buffer, &begin_info);

	return command_buffer;
}

void end_single_time_commands(_app *p_app, VkCommandBuffer command_buffer) {
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
	};

	vkQueueSubmit(p_app->device.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(p_app->device.graphics_queue);

	vkFreeCommandBuffers(p_app->device.logical, p_app->cmd.pool, 1, &command_buffer);
}
