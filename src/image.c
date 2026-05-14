#include "headers/image.h"
#include "headers/validation.h"
#include "headers/buffer.h"

#define IMAGE_FORMAT VK_FORMAT_R8G8B8A8_SRGB

void create_image(_app *p_app, VkImage *p_image, VmaAllocation *p_allocation, u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memory_usage) {
	VkImageCreateInfo image_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent = { .width = width, .height = height, .depth = 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.format = format,
		.tiling = tiling,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.usage = usage,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	VmaAllocationCreateInfo alloc_create_info = {
		.usage = memory_usage,
	};

	if (vmaCreateImage(p_app->mem.alloc, &image_create_info, &alloc_create_info, p_image, p_allocation, NULL) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"image => failed to create image"
		);
		exit(EXIT_FAILURE);
	}
}

void transition_image_layout(_app *p_app, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout) {
	VkCommandBuffer command_buffer = begin_single_time_commands(p_app);

	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	VkPipelineStageFlags source_stage;
	VkPipelineStageFlags destination_stage;

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"layout transition => unsupported layout transition"
		);
		exit(EXIT_FAILURE);
	}

	vkCmdPipelineBarrier(
		command_buffer,
		source_stage, destination_stage,
		0,
		0, NULL,
		0, NULL,
		1, &barrier
	);

	end_single_time_commands(p_app, command_buffer);
}

void copy_buffer_to_image(_app *p_app, VkBuffer buffer, VkImage image, u32 width, u32 height) {
	VkCommandBuffer command_buffer = begin_single_time_commands(p_app);

	VkBufferImageCopy region = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = {0, 0, 0},
		.imageExtent = {width, height, 1},
	};

	vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	end_single_time_commands(p_app, command_buffer);
}

static void create_sampler(_app *p_app, VkSampler *p_sampler) {
	VkSamplerCreateInfo sampler_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.anisotropyEnable = VK_FALSE,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	};

	if (vkCreateSampler(p_app->device.logical, &sampler_info, NULL, p_sampler) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"sampler => failed to create sampler"
		);
		exit(EXIT_FAILURE);
	}
}

static void create_image_view_simple(_app *p_app, VkImage image, VkImageView *p_view, VkFormat format) {
	VkImageViewCreateInfo view_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	if (vkCreateImageView(p_app->device.logical, &view_info, NULL, p_view) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"image view => failed to create image view"
		);
		exit(EXIT_FAILURE);
	}
}

static void write_image_descriptors(_app *p_app) {
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorImageInfo image_info = {
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.imageView = p_app->image.image_view,
			.sampler = p_app->image.sampler,
		};

		VkWriteDescriptorSet descriptor_write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = p_app->descriptor.sets[i],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.pImageInfo = &image_info,
		};

		vkUpdateDescriptorSets(p_app->device.logical, 1, &descriptor_write, 0, NULL);
	}
}

void create_image_resources(_app *p_app, u32 width, u32 height) {
	p_app->image.width = width;
	p_app->image.height = height;
	p_app->image.staging_size = (VkDeviceSize)width * height * 4;

	create_image(
		p_app,
		&p_app->image.image,
		&p_app->image.image_allocation,
		width, height,
		IMAGE_FORMAT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY
	);

	create_image_view_simple(p_app, p_app->image.image, &p_app->image.image_view, IMAGE_FORMAT);
	create_sampler(p_app, &p_app->image.sampler);

	VkBufferCreateInfo buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = p_app->image.staging_size,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	VmaAllocationCreateInfo alloc_create_info = {
		.usage = VMA_MEMORY_USAGE_AUTO,
		.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
	};

	VmaAllocationInfo allocation_info;
	if (vmaCreateBuffer(p_app->mem.alloc, &buffer_create_info, &alloc_create_info,
										 &p_app->image.staging_buffer,
										 &p_app->image.staging_allocation,
										 &allocation_info) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"staging buffer => failed to create staging buffer"
		);
		exit(EXIT_FAILURE);
	}

	p_app->image.staging_mapped = allocation_info.pMappedData;
	p_app->image.created = true;

	write_image_descriptors(p_app);
}

void destroy_image_resources(_app *p_app) {
	if (!p_app->image.created) return;

	vkDeviceWaitIdle(p_app->device.logical);

	vmaDestroyBuffer(p_app->mem.alloc, p_app->image.staging_buffer, p_app->image.staging_allocation);
	vkDestroySampler(p_app->device.logical, p_app->image.sampler, NULL);
	vkDestroyImageView(p_app->device.logical, p_app->image.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->image.image, p_app->image.image_allocation);

	p_app->image.staging_buffer = VK_NULL_HANDLE;
	p_app->image.staging_allocation = VK_NULL_HANDLE;
	p_app->image.staging_mapped = NULL;
	p_app->image.sampler = VK_NULL_HANDLE;
	p_app->image.image_view = VK_NULL_HANDLE;
	p_app->image.image = VK_NULL_HANDLE;
	p_app->image.image_allocation = VK_NULL_HANDLE;
	p_app->image.created = false;
}

void set_image_data(_app *p_app) {
	u32 width = p_app->output.width;
	u32 height = p_app->output.height;
	if (p_app->image.created && (p_app->image.width != width || p_app->image.height != height)) {
		destroy_image_resources(p_app);
	}

	if (!p_app->image.created) {
		create_image_resources(p_app, width, height);
	} else {
		vkDeviceWaitIdle(p_app->device.logical);
	}

	memcpy(p_app->image.staging_mapped, p_app->output.pixels, (size_t)width * height * 4);

	VkCommandBuffer cb = begin_single_time_commands(p_app);

	VkImageMemoryBarrier to_dst = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = p_app->image.image,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0, .levelCount = 1,
			.baseArrayLayer = 0, .layerCount = 1,
		},
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
	};
	vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &to_dst);

	VkBufferImageCopy region = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = {0, 0, 0},
		.imageExtent = {width, height, 1},
	};
	vkCmdCopyBufferToImage(cb, p_app->image.staging_buffer, p_app->image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	VkImageMemoryBarrier to_shader = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = p_app->image.image,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0, .levelCount = 1,
			.baseArrayLayer = 0, .layerCount = 1,
		},
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
	};
	vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &to_shader);

	end_single_time_commands(p_app, cb);
}
