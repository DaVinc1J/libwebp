#include "headers/swapchain.h"
#include "headers/validation.h"
#include "headers/core.h"
#include "headers/image.h"
#include "headers/lens.h"

VkSurfaceFormatKHR choose_swapchain_surface_format(_app *p_app, _swapchain_support *p_support) {

	for (u32 i = 0; i < p_support->surface_formats_count; i++) {
		if (p_support->surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && p_support->surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return p_support->surface_formats[i];
		}
	}

	submit_debug_message(
		p_app->inst.instance,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
		"swapchain surface format => desired surface format not found, falling back to first specified surface format"
	);
	return p_support->surface_formats[0];
}

VkPresentModeKHR choose_swapchain_present_mode(_app *p_app, _swapchain_support *p_support) {
	for (u32 i = 0; i  < p_support->present_modes_count; i++) {
		if (p_support->present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return p_support->present_modes[i];
		}
	}

	submit_debug_message(
		p_app->inst.instance,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
		"swapchain present mode => mailbox present mode not found, falling back to fifo"
	);
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_swapchain_swap_extent(_app *p_app, _swapchain_support *p_support) {
	if (p_support->capabilities.currentExtent.width != UINT32_MAX) {
		return p_support->capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(p_app->win.window, &width, &height);

		VkExtent2D actual_extent = { (u32)width, (u32)height };

		actual_extent.width = clamp(actual_extent.width, p_support->capabilities.minImageExtent.width, p_support->capabilities.maxImageExtent.width);
		actual_extent.height = clamp(actual_extent.height, p_support->capabilities.minImageExtent.height, p_support->capabilities.maxImageExtent.height);

		return actual_extent;
	}
}

//////// create swapchain ////////
//////////////////////////////////
void create_swapchain(_app *p_app) {
	_swapchain_support support;
	check_swapchain_support(p_app, p_app->device.physical, &support);

	p_app->swp.surface_format = choose_swapchain_surface_format(p_app, &support);
	VkPresentModeKHR present_mode = choose_swapchain_present_mode(p_app, &support);
	p_app->swp.extent = choose_swapchain_swap_extent(p_app, &support);

	float modifier = p_app->config.win.render_extent_modifier;
	p_app->swp.render_extent.width  = (u32)(p_app->swp.extent.width  * modifier);
	p_app->swp.render_extent.height = (u32)(p_app->swp.extent.height * modifier);
	if (p_app->swp.render_extent.width  == 0) p_app->swp.render_extent.width  = 1;
	if (p_app->swp.render_extent.height == 0) p_app->swp.render_extent.height = 1;

	u32 image_count = support.capabilities.minImageCount + 1;
	if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount) {
		image_count = support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = p_app->swp.surface,
		.minImageCount = image_count,
		.imageFormat = p_app->swp.surface_format.format,
		.imageColorSpace = p_app->swp.surface_format.colorSpace,
		.imageExtent = p_app->swp.extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.preTransform = support.capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE,
	};

	u32 queue_family_indices[] = {p_app->device.queue_indices.graphics_family, p_app->device.queue_indices.present_family};

	if (p_app->device.queue_indices.graphics_family != p_app->device.queue_indices.present_family) {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0;
		swapchain_create_info.pQueueFamilyIndices = NULL;
	}

	if (vkCreateSwapchainKHR(p_app->device.logical, &swapchain_create_info, NULL, &p_app->swp.swapchain) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"swapchain => failed to create swapchain"
		);
	}

	p_app->swp.images_count = 0;
	vkGetSwapchainImagesKHR(p_app->device.logical, p_app->swp.swapchain, &p_app->swp.images_count, NULL);
	p_app->swp.images = malloc(sizeof(VkImage) * p_app->swp.images_count);
	vkGetSwapchainImagesKHR(p_app->device.logical, p_app->swp.swapchain, &p_app->swp.images_count, p_app->swp.images);
}

//////// create image views ////////
////////////////////////////////////

void create_image_view(_app *p_app, VkImage image, VkImageView *p_image_view, u32 mip_levels, VkFormat format, VkImageAspectFlags aspect_flags) {
	VkImageViewCreateInfo view_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = {
			.aspectMask = aspect_flags,
			.baseMipLevel = 0,
			.levelCount = mip_levels,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	if (vkCreateImageView(p_app->device.logical, &view_create_info, NULL, p_image_view) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"image view => failed to create image view"
		);
		exit(EXIT_FAILURE);
	}
}

void create_image_views(_app *p_app) {
	p_app->swp.image_views = malloc(sizeof(VkImageView) * p_app->swp.images_count);

	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		create_image_view(
			p_app,
			p_app->swp.images[i],
			&p_app->swp.image_views[i],
			1,
			p_app->swp.surface_format.format,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
	}
}

void create_framebuffers(_app *p_app) {
	p_app->pipeline.swapchain_framebuffers = malloc(sizeof(VkFramebuffer) * p_app->swp.images_count);

	VkImageView attachments[] = {
		p_app->colour.image_view,
		p_app->depth.image_view,
		p_app->resolve.image_view,
	};

	VkFramebufferCreateInfo framebuffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = p_app->pipeline.render_pass,
		.width = p_app->swp.render_extent.width,
		.height = p_app->swp.render_extent.height,
		.layers = 1,
		.attachmentCount = sizeof(attachments) / sizeof(attachments[0]),
		.pAttachments = attachments,
	};

	for (size_t i = 0; i < p_app->swp.images_count; i++) {
		if (vkCreateFramebuffer(p_app->device.logical, &framebuffer_create_info, NULL, &p_app->pipeline.swapchain_framebuffers[i]) != VK_SUCCESS) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"framebuffers => failed to create framebuffers"
			);
			exit(EXIT_FAILURE);
		}
	}
}

void cleanup_swapchain(_app *p_app) {
	for (size_t i = 0; i < p_app->swp.images_count; i++) {
		vkDestroyFramebuffer(p_app->device.logical, p_app->pipeline.swapchain_framebuffers[i], NULL);
		vkDestroyImageView(p_app->device.logical, p_app->swp.image_views[i], NULL);
	}
	free(p_app->pipeline.swapchain_framebuffers);
	free(p_app->swp.image_views);


	vkDestroyImageView(p_app->device.logical, p_app->depth.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->depth.image, p_app->depth.image_allocation);
	vkDestroyImageView(p_app->device.logical, p_app->colour.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->colour.image, p_app->colour.image_allocation);
	vkDestroyImageView(p_app->device.logical, p_app->resolve.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->resolve.image, p_app->resolve.image_allocation);

	vkDestroySwapchainKHR(p_app->device.logical, p_app->swp.swapchain, NULL);
}

void recreate_swapchain(_app *p_app) {
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(p_app->win.window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(p_app->device.logical);

	cleanup_lens_swapchain(p_app);
	cleanup_swapchain(p_app);

	create_swapchain(p_app);
	create_image_views(p_app);
	create_colour_resources(p_app);
	create_depth_resources(p_app);
	create_resolve_resources(p_app);
	create_framebuffers(p_app);
	recreate_lens_swapchain(p_app);
}
