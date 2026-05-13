#include "headers/lens.h"
#include "headers/validation.h"
#include "headers/swapchain.h"
#include "headers/image.h"
#include "headers/pipeline.h"

void create_lens_render_pass(_app *p_app) {
	VkAttachmentDescription colour = {
		.format = p_app->swp.surface_format.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	};

	VkAttachmentReference colour_ref = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colour_ref,
	};

	VkSubpassDependency dependencies[] = {
		{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
		},
		{
			.srcSubpass = 0,
			.dstSubpass = VK_SUBPASS_EXTERNAL,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
			.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
		},
	};

	VkRenderPassCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colour,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = sizeof(dependencies) / sizeof(dependencies[0]),
		.pDependencies = dependencies,
	};

	if (vkCreateRenderPass(p_app->device.logical, &info, NULL, &p_app->lens.pass.render_pass) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "lens render pass => failed to create");
		exit(EXIT_FAILURE);
	}
}

void create_lens_image(_app *p_app) {
	VkFormat format = p_app->swp.surface_format.format;
	create_image(
		p_app, &p_app->lens.target.image, 1, VK_SAMPLE_COUNT_1_BIT,
		&p_app->lens.target.image_allocation,
		p_app->swp.render_extent.width, p_app->swp.render_extent.height,
		format, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY
	);
	create_image_view(p_app, p_app->lens.target.image, &p_app->lens.target.image_view, 1, format, VK_IMAGE_ASPECT_COLOR_BIT);
}

void create_lens_framebuffer(_app *p_app) {
	VkFramebufferCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = p_app->lens.pass.render_pass,
		.attachmentCount = 1,
		.pAttachments = &p_app->lens.target.image_view,
		.width = p_app->swp.render_extent.width,
		.height = p_app->swp.render_extent.height,
		.layers = 1,
	};
	if (vkCreateFramebuffer(p_app->device.logical, &info, NULL, &p_app->lens.pass.framebuffer) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "lens framebuffer => failed to create");
		exit(EXIT_FAILURE);
	}
}

void create_lens_sampler(_app *p_app) {
	VkSamplerCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		.minLod = 0.0f, .maxLod = 0.0f,
	};
	if (vkCreateSampler(p_app->device.logical, &info, NULL, &p_app->lens.target.sampler) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "lens sampler => failed to create");
		exit(EXIT_FAILURE);
	}
}

void create_lens_descriptor_set_layout(_app *p_app) {
	VkDescriptorSetLayoutBinding bindings[] = {
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		},
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		},
		{
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		},
	};

	VkDescriptorSetLayoutCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = sizeof(bindings) / sizeof(bindings[0]),
		.pBindings = bindings,
	};

	if (vkCreateDescriptorSetLayout(p_app->device.logical, &info, NULL, &p_app->lens.pass.descriptor_set_layout) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "lens descriptor set layout => failed to create");
		exit(EXIT_FAILURE);
	}
}

void create_lens_descriptor_pool(_app *p_app) {
	VkDescriptorPoolSize sizes[] = {
		{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = MAX_FRAMES_IN_FLIGHT },
		{ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = MAX_FRAMES_IN_FLIGHT },
		{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = MAX_FRAMES_IN_FLIGHT },
	};

	VkDescriptorPoolCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = sizeof(sizes) / sizeof(sizes[0]),
		.pPoolSizes = sizes,
		.maxSets = MAX_FRAMES_IN_FLIGHT,
	};

	if (vkCreateDescriptorPool(p_app->device.logical, &info, NULL, &p_app->lens.descriptor.pool) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "lens descriptor pool => failed to create");
		exit(EXIT_FAILURE);
	}
}

void write_lens_descriptor_sets(_app *p_app) {
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo ubo_info = {
			.buffer = p_app->uniform.buffers[i],
			.offset = 0,
			.range = VK_WHOLE_SIZE,
		};
		VkDescriptorBufferInfo sbo_info = {
			.buffer = p_app->storage.solar_object_buffers[i],
			.offset = 0,
			.range = VK_WHOLE_SIZE,
		};
		VkDescriptorImageInfo img_info = {
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.imageView = p_app->resolve.image_view,
			.sampler = p_app->lens.target.sampler,
		};

		VkWriteDescriptorSet writes[] = {
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = p_app->lens.descriptor.sets[i],
				.dstBinding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.pBufferInfo = &ubo_info,
			},
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = p_app->lens.descriptor.sets[i],
				.dstBinding = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.descriptorCount = 1,
				.pBufferInfo = &sbo_info,
			},
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = p_app->lens.descriptor.sets[i],
				.dstBinding = 2,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = 1,
				.pImageInfo = &img_info,
			},
		};

		vkUpdateDescriptorSets(p_app->device.logical, sizeof(writes) / sizeof(writes[0]), writes, 0, NULL);
	}
}

void create_lens_descriptor_sets(_app *p_app) {
	p_app->lens.descriptor.sets = malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);
	VkDescriptorSetLayout *layouts = malloc(sizeof(VkDescriptorSetLayout) * MAX_FRAMES_IN_FLIGHT);
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) layouts[i] = p_app->lens.pass.descriptor_set_layout;

	VkDescriptorSetAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = p_app->lens.descriptor.pool,
		.descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
		.pSetLayouts = layouts,
	};

	if (vkAllocateDescriptorSets(p_app->device.logical, &alloc_info, p_app->lens.descriptor.sets) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "lens descriptor sets => failed to allocate");
		exit(EXIT_FAILURE);
	}
	free(layouts);

	write_lens_descriptor_sets(p_app);
}

void create_lens_pipeline(_app *p_app) {
	size_t vert_size, frag_size;
	const char *vert_code = read_file(p_app, p_app->shader.lens_vert, &vert_size);
	const char *frag_code = read_file(p_app, p_app->shader.lens_frag, &frag_size);

	VkShaderModule vert = create_shader_module(p_app, vert_code, vert_size);
	VkShaderModule frag = create_shader_module(p_app, frag_code, frag_size);

	VkPipelineShaderStageCreateInfo stages[2] = {
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vert, .pName = "main" },
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = frag, .pName = "main" },
	};

	VkPipelineVertexInputStateCreateInfo vertex_input = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	VkPipelineInputAssemblyStateCreateInfo input_asm = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	VkViewport viewport = {
		.width = (float)p_app->swp.render_extent.width,
		.height = (float)p_app->swp.render_extent.height,
		.minDepth = 0.0f, .maxDepth = 1.0f,
	};
	VkRect2D scissor = { .extent = p_app->swp.render_extent };

	VkPipelineViewportStateCreateInfo viewport_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1, .pViewports = &viewport,
		.scissorCount = 1, .pScissors = &scissor,
	};

	VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamic_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamic_states,
	};

	VkPipelineRasterizationStateCreateInfo raster = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.lineWidth = 1.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisample = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	};

	VkPipelineDepthStencilStateCreateInfo depth = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	};

	VkPipelineColorBlendAttachmentState blend = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_FALSE,
	};
	VkPipelineColorBlendStateCreateInfo blend_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &blend,
	};

	VkPipelineLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &p_app->lens.pass.descriptor_set_layout,
	};

	if (vkCreatePipelineLayout(p_app->device.logical, &layout_info, NULL, &p_app->lens.pass.layout) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "lens pipeline layout => failed");
		exit(EXIT_FAILURE);
	}

	VkGraphicsPipelineCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = stages,
		.pVertexInputState = &vertex_input,
		.pInputAssemblyState = &input_asm,
		.pViewportState = &viewport_state,
		.pRasterizationState = &raster,
		.pMultisampleState = &multisample,
		.pDepthStencilState = &depth,
		.pColorBlendState = &blend_state,
		.pDynamicState = &dynamic_state,
		.layout = p_app->lens.pass.layout,
		.renderPass = p_app->lens.pass.render_pass,
	};

	if (vkCreateGraphicsPipelines(p_app->device.logical, VK_NULL_HANDLE, 1, &info, NULL, &p_app->lens.pass.pipeline) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "lens pipeline => failed");
		exit(EXIT_FAILURE);
	}

	vkDestroyShaderModule(p_app->device.logical, vert, NULL);
	vkDestroyShaderModule(p_app->device.logical, frag, NULL);
	free((void*)vert_code);
	free((void*)frag_code);
}

void cleanup_lens_swapchain(_app *p_app) {
	vkDestroyFramebuffer(p_app->device.logical, p_app->lens.pass.framebuffer, NULL);
	vkDestroyImageView(p_app->device.logical, p_app->lens.target.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->lens.target.image, p_app->lens.target.image_allocation);
}

void recreate_lens_swapchain(_app *p_app) {
	create_lens_image(p_app);
	create_lens_framebuffer(p_app);
	write_lens_descriptor_sets(p_app);
}
