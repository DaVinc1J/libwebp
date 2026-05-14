#include "headers/loop.h"
#include "headers/validation.h"
#include "headers/swapchain.h"
#include "headers/buffer.h"

void log_performance(_app *p_app) {
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	if (p_app->perf.frame_count == 0) {
		p_app->perf.last_frame_time = now;
		p_app->perf.frame_count++;
		return;
	}

	double dt = (now.tv_sec - p_app->perf.last_frame_time.tv_sec) +
		(now.tv_nsec - p_app->perf.last_frame_time.tv_nsec) / 1e9;

	p_app->perf.last_frame_time = now;

	p_app->perf.frame_time_avg += (dt - p_app->perf.frame_time_avg) * 0.05;
	p_app->perf.fps_avg = 1.0f / p_app->perf.frame_time_avg;

	p_app->perf.frame_count++;
	if (p_app->config.print_fps && p_app->perf.frame_count % 60 == 0) {
		printf("[perf] FPS: %.1f, Frame Time: %.2f ms\n", p_app->perf.fps_avg, p_app->perf.frame_time_avg * 1000.0f);
	}
}

void draw_frame(_app *p_app) {
	vkWaitForFences(p_app->device.logical, 1, &p_app->sync.in_flight_fences[p_app->sync.frame_index], VK_TRUE, UINT64_MAX);

	u32 image_index;
	VkResult aquire_result = vkAcquireNextImageKHR(
		p_app->device.logical,
		p_app->swp.swapchain,
		UINT64_MAX,
		p_app->sync.image_available_semaphores[p_app->sync.frame_index],
		VK_NULL_HANDLE,
		&image_index
	);

	if (aquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swapchain(p_app);
		return;
	} else if (aquire_result != VK_SUCCESS && aquire_result != VK_SUBOPTIMAL_KHR) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"swapchain => failed to acquire swap chain image"
		);
		exit(EXIT_FAILURE);
	}

	vkResetFences(p_app->device.logical, 1, &p_app->sync.in_flight_fences[p_app->sync.frame_index]);

	vkResetCommandBuffer(p_app->cmd.buffers[p_app->sync.frame_index], 0);
	record_command_buffer(p_app, p_app->cmd.buffers[p_app->sync.frame_index], image_index);

	VkSemaphore wait_semaphores[] = {p_app->sync.image_available_semaphores[p_app->sync.frame_index]};
	VkSemaphore signal_semaphores[] = {p_app->sync.render_finished_semaphores[image_index]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait_semaphores,
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &p_app->cmd.buffers[p_app->sync.frame_index],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signal_semaphores,
	};

	if (vkQueueSubmit(p_app->device.graphics_queue, 1, &submit_info, p_app->sync.in_flight_fences[p_app->sync.frame_index]) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"graphics queue => failed to submit next queue"
		);
		exit(EXIT_FAILURE);
	}

	VkSwapchainKHR swapchains[] = {p_app->swp.swapchain};
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signal_semaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &image_index,
		.pResults = NULL,
	};

	VkResult present_result = vkQueuePresentKHR(p_app->device.present_queue, &present_info);

	if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR || p_app->swp.framebuffer_resized) {
		p_app->swp.framebuffer_resized = false;
		recreate_swapchain(p_app);
		return;
	} else if (present_result != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"swapchain => failed to present swap chain image"
		);
		exit(EXIT_FAILURE);
	}

	p_app->sync.frame_index = (p_app->sync.frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}
