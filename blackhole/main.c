#define FAST_OBJ_IMPLEMENTATION
#define CGLTF_IMPLEMENTATION
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
#include "headers/object.h"
#include "headers/lens.h"

void vulkan_init(_app *p_app);
void clean(_app *p_app);
void main_loop(_app *p_app);

const u32 MAX_FRAMES_IN_FLIGHT = 2;

u32 clamp(u32 n, u32 min, u32 max) {
	if (n < min) return min;
	if (n > max) return max;
	return n;
}

int main() {
	_app app = {0};
	app_init(&app);
	window_init(&app);
	vulkan_init(&app);
	main_loop(&app);
	clean(&app);
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
	create_graphics_pipelines(p_app);
	create_command_pool(p_app);
	create_colour_resources(p_app);
	create_depth_resources(p_app);
	create_resolve_resources(p_app);
	create_framebuffers(p_app);
	create_billboards(p_app);
	create_billboard_buffer(p_app);
	create_grid_lines(p_app);
	create_grid_buffer(p_app);
	create_spheres(p_app);
	create_mesh_buffer(p_app);
	create_uniform_buffers(p_app);
	create_storage_buffers(p_app);
	create_descriptor_pool(p_app);
	create_descriptor_sets(p_app);
	create_lens_render_pass(p_app);
	create_lens_image(p_app);
	create_lens_framebuffer(p_app);
	create_lens_sampler(p_app);
	create_lens_descriptor_set_layout(p_app);
	create_lens_pipeline(p_app);
	create_lens_descriptor_pool(p_app);
	create_lens_descriptor_sets(p_app);
	create_command_buffers(p_app);
	create_sync_objects(p_app);
}

void main_loop(_app *p_app) {

	while (!glfwWindowShouldClose(p_app->win.window)) {
		glfwPollEvents();
		log_performance(p_app);
		update_view(p_app, get_delta_time());
		draw_frame(p_app);
	}

	vkDeviceWaitIdle(p_app->device.logical);
}

void clean(_app *p_app) {
	vkDestroyImageView(p_app->device.logical, p_app->depth.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->depth.image, p_app->depth.image_allocation);
	vkDestroyImageView(p_app->device.logical, p_app->colour.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->colour.image, p_app->colour.image_allocation);
	vkDestroyImageView(p_app->device.logical, p_app->resolve.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->resolve.image, p_app->resolve.image_allocation);
	vkDestroyImageView(p_app->device.logical, p_app->lens.target.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->lens.target.image, p_app->lens.target.image_allocation);

	vkDestroyDescriptorPool(p_app->device.logical, p_app->descriptor.pool, NULL);
	p_app->descriptor.pool = VK_NULL_HANDLE;
	vkDestroyDescriptorSetLayout(p_app->device.logical, p_app->pipeline.descriptor_set_layout, NULL);
	p_app->pipeline.descriptor_set_layout = VK_NULL_HANDLE;
	free(p_app->descriptor.sets);
	p_app->descriptor.sets = NULL;

	vkDestroyDescriptorPool(p_app->device.logical, p_app->lens.descriptor.pool, NULL);
	p_app->lens.descriptor.pool = VK_NULL_HANDLE;
	vkDestroyDescriptorSetLayout(p_app->device.logical, p_app->lens.pass.descriptor_set_layout, NULL);
	p_app->lens.pass.descriptor_set_layout = VK_NULL_HANDLE;
	free(p_app->lens.descriptor.sets);
	p_app->lens.descriptor.sets = NULL;

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vmaDestroyBuffer(p_app->mem.alloc, p_app->uniform.buffers[i], p_app->uniform.buffer_allocations[i]);
	}
	free(p_app->uniform.buffers);
	p_app->uniform.buffers = NULL;
	free(p_app->uniform.buffer_allocations);
	p_app->uniform.buffer_allocations = NULL;
	free(p_app->uniform.buffers_mapped);
	p_app->uniform.buffers_mapped = NULL;

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vmaDestroyBuffer(
			p_app->mem.alloc,
			p_app->storage.billboard_buffers[i],
			p_app->storage.billboard_buffer_allocations[i]
		);
	}

	free(p_app->storage.billboard_buffers);
	p_app->storage.billboard_buffers = NULL;
	free(p_app->storage.billboard_buffer_allocations);
	p_app->storage.billboard_buffer_allocations = NULL;
	free(p_app->storage.billboard_buffers_mapped);
	p_app->storage.billboard_buffers_mapped = NULL;

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vmaDestroyBuffer(
			p_app->mem.alloc,
			p_app->storage.solar_object_buffers[i],
			p_app->storage.solar_object_buffer_allocations[i]
		);
	}

	free(p_app->storage.solar_object_buffers);
	p_app->storage.solar_object_buffers = NULL;
	free(p_app->storage.solar_object_buffer_allocations);
	p_app->storage.solar_object_buffer_allocations = NULL;
	free(p_app->storage.solar_object_buffers_mapped);
	p_app->storage.solar_object_buffers_mapped = NULL;

	for (u32 i = 0; i < MESH_SPHERE_LOD_COUNT; i++) {
		vmaDestroyBuffer(p_app->mem.alloc, p_app->mesh.index_buffers[i], p_app->mesh.index_allocations[i]);
		vmaDestroyBuffer(p_app->mem.alloc, p_app->mesh.vertex_buffers[i], p_app->mesh.vertex_allocations[i]);
	}

	free(p_app->mesh.index_buffers);
	p_app->mesh.index_buffers = NULL;
	free(p_app->mesh.index_allocations);
	p_app->mesh.index_allocations = NULL;
	free(p_app->mesh.vertex_buffers);
	p_app->mesh.vertex_buffers = NULL;
	free(p_app->mesh.vertex_allocations);
	p_app->mesh.vertex_allocations = NULL;

	vmaDestroyBuffer(p_app->mem.alloc, p_app->billboard.instance_buffer, p_app->billboard.instance_allocation);

	vmaDestroyBuffer(p_app->mem.alloc, p_app->grid.vertex_buffer, p_app->grid.vertex_allocation);

	free(p_app->cmd.buffers);
	p_app->cmd.buffers = NULL;

	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		vkDestroySemaphore(p_app->device.logical, p_app->sync.image_available_semaphores[i], NULL);
		vkDestroySemaphore(p_app->device.logical, p_app->sync.render_finished_semaphores[i], NULL);
	}
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyFence(p_app->device.logical, p_app->sync.in_flight_fences[i], NULL);
	}
	free(p_app->sync.image_available_semaphores);
	p_app->sync.image_available_semaphores = NULL;
	free(p_app->sync.render_finished_semaphores);
	p_app->sync.render_finished_semaphores = NULL;
	free(p_app->sync.in_flight_fences);
	p_app->sync.in_flight_fences = NULL;

	vkDestroyCommandPool(p_app->device.logical, p_app->cmd.pool, NULL);
	p_app->cmd.pool = VK_NULL_HANDLE;

	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		vkDestroyFramebuffer(p_app->device.logical, p_app->pipeline.swapchain_framebuffers[i], NULL);
	}
	free(p_app->pipeline.swapchain_framebuffers);
	p_app->pipeline.swapchain_framebuffers = NULL;

	vkDestroyFramebuffer(p_app->device.logical, p_app->lens.pass.framebuffer, NULL);

	vkDestroyPipeline(p_app->device.logical, p_app->pipeline.opaque, NULL);
	p_app->pipeline.opaque = VK_NULL_HANDLE;
	vkDestroyPipeline(p_app->device.logical, p_app->pipeline.transparent, NULL);
	p_app->pipeline.transparent = VK_NULL_HANDLE;
	vkDestroyPipeline(p_app->device.logical, p_app->pipeline.billboard, NULL);
	p_app->pipeline.billboard = VK_NULL_HANDLE;
	vkDestroyPipeline(p_app->device.logical, p_app->pipeline.grid, NULL);
	p_app->pipeline.grid = VK_NULL_HANDLE;
	vkDestroyPipelineLayout(p_app->device.logical, p_app->pipeline.layout, NULL);
	p_app->pipeline.layout = VK_NULL_HANDLE;
	vkDestroyRenderPass(p_app->device.logical, p_app->pipeline.render_pass, NULL);
	p_app->pipeline.render_pass = VK_NULL_HANDLE;

	vkDestroyPipeline(p_app->device.logical, p_app->lens.pass.pipeline, NULL);
	p_app->lens.pass.pipeline = VK_NULL_HANDLE;
	vkDestroyPipelineLayout(p_app->device.logical, p_app->lens.pass.layout, NULL);
	p_app->lens.pass.layout = VK_NULL_HANDLE;
	vkDestroySampler(p_app->device.logical, p_app->lens.target.sampler, NULL);
	p_app->lens.target.sampler = VK_NULL_HANDLE;
	vkDestroyRenderPass(p_app->device.logical, p_app->lens.pass.render_pass, NULL);
	p_app->lens.pass.render_pass = VK_NULL_HANDLE;

	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		vkDestroyImageView(p_app->device.logical, p_app->swp.image_views[i], NULL);
	}
	free(p_app->swp.image_views);
	p_app->swp.image_views = NULL;
	vkDestroySwapchainKHR(p_app->device.logical, p_app->swp.swapchain, NULL);
	p_app->swp.swapchain = VK_NULL_HANDLE;

	vmaDestroyAllocator(p_app->mem.alloc);
	vkDestroyDevice(p_app->device.logical, NULL);
	p_app->device.logical = VK_NULL_HANDLE;

	if (enable_validation_layers) {
		destroy_debug_utils_messenger_EXT(p_app->inst.instance, p_app->inst.debug_messenger, NULL);
	}
	vkDestroySurfaceKHR(p_app->inst.instance, p_app->swp.surface, NULL);
	p_app->swp.surface = VK_NULL_HANDLE;
	vkDestroyInstance(p_app->inst.instance, NULL);
	p_app->inst.instance = VK_NULL_HANDLE;

	glfwDestroyWindow(p_app->win.window);
	glfwTerminate();
}
