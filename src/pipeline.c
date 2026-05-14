#include "headers/pipeline.h"
#include "headers/validation.h"

char* read_file(_app *p_app, const char* filename, size_t* shader_code_size) {
	FILE* p_file = fopen(filename, "rb");
	if (!p_file) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"shader read => failed to open file"
		);
		return NULL;
	}

	fseek(p_file, 0, SEEK_END);
	long file_size = ftell(p_file);
	rewind(p_file);

	if (file_size <= 0) {
		fclose(p_file);
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"shader read => empty or invalid file"
		);
		return NULL;
	}

	char* buffer = malloc(file_size);

	fread(buffer, 1, file_size, p_file);
	fclose(p_file);

	*shader_code_size = (size_t)file_size;
	return buffer;
}

VkShaderModule create_shader_module(_app *p_app, const char* shader_code, size_t shader_code_size) {
	VkShaderModuleCreateInfo shader_module_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shader_code_size,
		.pCode = (const uint32_t*) shader_code,
	};

	VkShaderModule shader_module;
	if (vkCreateShaderModule(p_app->device.logical, &shader_module_create_info, NULL, &shader_module) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"shader module => failed to create shader module"
		);
		exit(EXIT_FAILURE);
	}

	return shader_module;
}

void create_graphics_pipeline(_app *p_app) {
	size_t vert_size, frag_size;
	char* vert_code = read_file(p_app, p_app->config.vert_shader, &vert_size);
	char* frag_code = read_file(p_app, p_app->config.frag_shader, &frag_size);

	VkShaderModule vert_module = create_shader_module(p_app, vert_code, vert_size);
	VkShaderModule frag_module = create_shader_module(p_app, frag_code, frag_size);

	VkPipelineShaderStageCreateInfo shader_stages[2] = {
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT,   .module = vert_module, .pName = "main" },
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = frag_module, .pName = "main" },
	};

	VkPipelineVertexInputStateCreateInfo vertex_input = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	VkPipelineInputAssemblyStateCreateInfo input_asm = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	VkPipelineViewportStateCreateInfo viewport_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
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

	VkPipelineColorBlendAttachmentState blend_attachment = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_FALSE,
	};

	VkPipelineColorBlendStateCreateInfo blend_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &blend_attachment,
	};

	VkPushConstantRange pc = {
    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    .offset = 0,
    .size = sizeof(_pc),
	};

	VkPipelineLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &p_app->pipeline.descriptor_set_layout,
		.pPushConstantRanges = &pc,
		.pushConstantRangeCount = 1,
	};

	if (vkCreatePipelineLayout(p_app->device.logical, &layout_info, NULL, &p_app->pipeline.layout) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "pipeline layout => failed");
		exit(EXIT_FAILURE);
	}

	VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shader_stages,
		.pVertexInputState = &vertex_input,
		.pInputAssemblyState = &input_asm,
		.pViewportState = &viewport_state,
		.pRasterizationState = &raster,
		.pMultisampleState = &multisample,
		.pDynamicState = &dynamic_state,
		.pColorBlendState = &blend_state,
		.layout = p_app->pipeline.layout,
		.renderPass = p_app->pipeline.render_pass,
	};

	if (vkCreateGraphicsPipelines(p_app->device.logical, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &p_app->pipeline.pipeline) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "graphics pipeline => failed");
		exit(EXIT_FAILURE);
	}

	vkDestroyShaderModule(p_app->device.logical, vert_module, NULL);
	vkDestroyShaderModule(p_app->device.logical, frag_module, NULL);
	free(vert_code);
	free(frag_code);
}
