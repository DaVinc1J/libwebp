#include "headers/define.h"
#include "headers/app.h"
#include "headers/window.h"
#include "headers/validation.h"
#include "headers/core.h"
#include "headers/swapchain.h"
#include "headers/renderpass.h"
#include "headers/pipeline.h"
#include "headers/sync.h"
#include "headers/buffer.h"
#include "headers/image.h"
#include "headers/descriptors.h"
#include "headers/loop.h"
#include "headers/file.h"

void vulkan_init(_app *p_app);
void file_init(_app *p_app, const char* argv);
void clean(_app *p_app);
void main_loop(_app *p_app);
static void load_test_pattern(_app *p_app);

const u32 MAX_FRAMES_IN_FLIGHT = 2;

u32 clamp(u32 n, u32 min, u32 max) {
	if (n < min) return min;
	if (n > max) return max;
	return n;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		exit(1);
	}
	_app app = {0};
	app_init(&app);
	window_init(&app);
	vulkan_init(&app);
	file_init(&app, argv[1]);
	//load_test_pattern(&app);
	main_loop(&app);
	clean(&app);
}

void file_init(_app *p_app, const char* argv) {
	file_setup(p_app, argv);
	file_read(p_app);
	file_decode(p_app);
}

void vulkan_init(_app *p_app) {
	create_instance(p_app);
	setup_debug_messenger(p_app);
	create_surface(p_app);
	pick_physical_device(p_app);
	create_logical_device(p_app);
	create_alloc(p_app);
	create_swapchain(p_app);
	create_image_views(p_app);
	create_render_pass(p_app);
	create_descriptor_set_layout(p_app);
	create_graphics_pipeline(p_app);
	create_command_pool(p_app);
	create_framebuffers(p_app);
	create_descriptor_pool(p_app);
	create_descriptor_sets(p_app);
	create_command_buffers(p_app);
	create_sync_objects(p_app);
}

static void load_test_pattern(_app *p_app) {
	p_app->output.width = 512;
	p_app->output.height = 512;
	p_app->output.pixels = malloc((size_t)p_app->output.width * p_app->output.height * 4);
	for (u32 y = 0; y < p_app->output.height; y++) {
		for (u32 x = 0; x < p_app->output.width; x++) {
			u8 *px = &(p_app->output.pixels)[(y * p_app->output.width + x) * 4];
			px[0] = (u8)x;
			px[1] = (u8)y;
			px[2] = (u8)((x ^ y) & 0xFF);
			px[3] = 255;
		}
	}
	set_image_data(p_app);
	free(p_app->output.pixels);
}

void main_loop(_app *p_app) {
	while (!glfwWindowShouldClose(p_app->win.window)) {
		glfwPollEvents();
		log_performance(p_app);
		draw_frame(p_app);
	}

	vkDeviceWaitIdle(p_app->device.logical);
}

void clean(_app *p_app) {
	destroy_image_resources(p_app);

	vkDestroyDescriptorPool(p_app->device.logical, p_app->descriptor.pool, NULL);
	vkDestroyDescriptorSetLayout(p_app->device.logical, p_app->pipeline.descriptor_set_layout, NULL);
	free(p_app->descriptor.sets);

	free(p_app->cmd.buffers);

	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		vkDestroySemaphore(p_app->device.logical, p_app->sync.image_available_semaphores[i], NULL);
		vkDestroySemaphore(p_app->device.logical, p_app->sync.render_finished_semaphores[i], NULL);
	}
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyFence(p_app->device.logical, p_app->sync.in_flight_fences[i], NULL);
	}
	free(p_app->sync.image_available_semaphores);
	free(p_app->sync.render_finished_semaphores);
	free(p_app->sync.in_flight_fences);

	vkDestroyCommandPool(p_app->device.logical, p_app->cmd.pool, NULL);

	cleanup_swapchain(p_app);

	vkDestroyPipeline(p_app->device.logical, p_app->pipeline.pipeline, NULL);
	vkDestroyPipelineLayout(p_app->device.logical, p_app->pipeline.layout, NULL);
	vkDestroyRenderPass(p_app->device.logical, p_app->pipeline.render_pass, NULL);

	vmaDestroyAllocator(p_app->mem.alloc);
	vkDestroyDevice(p_app->device.logical, NULL);

	if (enable_validation_layers) {
		destroy_debug_utils_messenger_EXT(p_app->inst.instance, p_app->inst.debug_messenger, NULL);
	}
	vkDestroySurfaceKHR(p_app->inst.instance, p_app->swp.surface, NULL);
	vkDestroyInstance(p_app->inst.instance, NULL);

	glfwDestroyWindow(p_app->win.window);
	glfwTerminate();
}
