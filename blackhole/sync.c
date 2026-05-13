#include "headers/sync.h"
#include "headers/validation.h"

void create_sync_objects(_app *p_app) {

	p_app->sync.image_available_semaphores = malloc(sizeof(VkSemaphore) * p_app->swp.images_count);
	p_app->sync.render_finished_semaphores = malloc(sizeof(VkSemaphore) * p_app->swp.images_count);
	p_app->sync.in_flight_fences = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphore_create_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	VkFenceCreateInfo fence_create_info = {	
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		if (vkCreateSemaphore(p_app->device.logical, &semaphore_create_info, NULL, &p_app->sync.image_available_semaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(p_app->device.logical, &semaphore_create_info, NULL, &p_app->sync.render_finished_semaphores[i]) != VK_SUCCESS) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"semaphore => failed to create semaphore"
			);
			exit(EXIT_FAILURE);
		}
	}

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateFence(p_app->device.logical, &fence_create_info, NULL, &p_app->sync.in_flight_fences[i]) != VK_SUCCESS) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"fence => failed to create fence"
			);
			exit(EXIT_FAILURE);
		}
	}
}
