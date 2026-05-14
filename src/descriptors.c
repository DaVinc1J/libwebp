#include "headers/descriptors.h"
#include "headers/validation.h"

void create_descriptor_set_layout(_app *p_app) {
	VkDescriptorSetLayoutBinding sampler_binding = {
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = NULL,
	};

	VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &sampler_binding,
	};

	if (vkCreateDescriptorSetLayout(p_app->device.logical, &descriptor_layout_create_info, NULL, &p_app->pipeline.descriptor_set_layout) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"descriptor set layout => failed to create descriptor set layout"
		);
		exit(EXIT_FAILURE);
	}
}

void create_descriptor_pool(_app *p_app) {
	VkDescriptorPoolSize pool_size = {
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = MAX_FRAMES_IN_FLIGHT,
	};

	VkDescriptorPoolCreateInfo pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = 1,
		.pPoolSizes = &pool_size,
		.maxSets = MAX_FRAMES_IN_FLIGHT,
	};

	if (vkCreateDescriptorPool(p_app->device.logical, &pool_create_info, NULL, &p_app->descriptor.pool) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"descriptor pool => failed to create descriptor pool"
		);
		exit(EXIT_FAILURE);
	}
}

void create_descriptor_sets(_app *p_app) {
	p_app->descriptor.sets = malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);
	VkDescriptorSetLayout *layouts = malloc(sizeof(VkDescriptorSetLayout) * MAX_FRAMES_IN_FLIGHT);

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		layouts[i] = p_app->pipeline.descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = p_app->descriptor.pool,
		.descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
		.pSetLayouts = layouts,
	};

	if (vkAllocateDescriptorSets(p_app->device.logical, &alloc_info, p_app->descriptor.sets) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"descriptor sets => failed to allocate descriptor sets"
		);
		exit(EXIT_FAILURE);
	}

	free(layouts);
}
