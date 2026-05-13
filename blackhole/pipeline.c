#include "headers/pipeline.h"
#include "headers/shader.h"
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

void create_graphics_pipelines(_app *p_app) {
	size_t mesh_vert_shader_code_size;
	size_t mesh_frag_shader_code_size;
	size_t billboard_vert_shader_code_size;
	size_t billboard_frag_shader_code_size;
	size_t grid_vert_shader_code_size;
	size_t grid_frag_shader_code_size;

	const char* mesh_vert_shader_code = read_file(p_app, p_app->shader.mesh_vert, &mesh_vert_shader_code_size);
	const char* mesh_frag_shader_code = read_file(p_app, p_app->shader.mesh_frag, &mesh_frag_shader_code_size);
	const char* billboard_vert_shader_code = read_file(p_app, p_app->shader.billboard_vert, &billboard_vert_shader_code_size);
	const char* billboard_frag_shader_code = read_file(p_app, p_app->shader.billboard_frag, &billboard_frag_shader_code_size);
	const char* grid_vert_shader_code = read_file(p_app, p_app->shader.grid_vert, &grid_vert_shader_code_size);
	const char* grid_frag_shader_code = read_file(p_app, p_app->shader.grid_frag, &grid_frag_shader_code_size);

	VkShaderModule mesh_vert_shader_module = create_shader_module(p_app, mesh_vert_shader_code, mesh_vert_shader_code_size); 
	VkShaderModule mesh_frag_shader_module = create_shader_module(p_app, mesh_frag_shader_code, mesh_frag_shader_code_size);
	VkShaderModule billboard_vert_shader_module = create_shader_module(p_app, billboard_vert_shader_code, billboard_vert_shader_code_size); 
	VkShaderModule billboard_frag_shader_module = create_shader_module(p_app, billboard_frag_shader_code, billboard_frag_shader_code_size);
	VkShaderModule grid_vert_shader_module = create_shader_module(p_app, grid_vert_shader_code, grid_vert_shader_code_size); 
	VkShaderModule grid_frag_shader_module = create_shader_module(p_app, grid_frag_shader_code, grid_frag_shader_code_size);

	VkPipelineShaderStageCreateInfo mesh_shader_stages[2] = {
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = mesh_vert_shader_module, .pName = "main" },
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = mesh_frag_shader_module, .pName = "main" },
	};

	VkPipelineShaderStageCreateInfo billboard_shader_stages[2] = {
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = billboard_vert_shader_module, .pName = "main" },
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = billboard_frag_shader_module, .pName = "main" },
	};

	VkPipelineShaderStageCreateInfo grid_shader_stages[2] = {
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = grid_vert_shader_module, .pName = "main" },
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = grid_frag_shader_module, .pName = "main" },
	};

	VkVertexInputBindingDescription mesh_binding_desc = get_mesh_binding_description();
	u32 mesh_attr_count = 0;
	get_mesh_attribute_descriptions(NULL, &mesh_attr_count);
	VkVertexInputAttributeDescription mesh_attr_descs[mesh_attr_count];
	get_mesh_attribute_descriptions(mesh_attr_descs, NULL);

	VkVertexInputBindingDescription billboard_binding_desc = get_billboard_binding_description();
	u32 billboard_attr_count = 0;
	get_billboard_attribute_descriptions(NULL, &billboard_attr_count);
	VkVertexInputAttributeDescription billboard_attr_descs[billboard_attr_count];
	get_billboard_attribute_descriptions(billboard_attr_descs, NULL);

	VkVertexInputBindingDescription grid_binding_desc = get_grid_binding_description();
	u32 grid_attr_count = 0;
	get_grid_attribute_descriptions(NULL, &grid_attr_count);
	VkVertexInputAttributeDescription grid_attr_descs[grid_attr_count];
	get_grid_attribute_descriptions(grid_attr_descs, NULL);

	VkPipelineVertexInputStateCreateInfo mesh_vertex_input = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.vertexAttributeDescriptionCount = mesh_attr_count,
		.pVertexBindingDescriptions = &mesh_binding_desc,
		.pVertexAttributeDescriptions = mesh_attr_descs,
	};

	VkPipelineVertexInputStateCreateInfo billboard_vertex_input = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.vertexAttributeDescriptionCount = billboard_attr_count,
		.pVertexBindingDescriptions = &billboard_binding_desc,
		.pVertexAttributeDescriptions = billboard_attr_descs,
	};

	VkPipelineVertexInputStateCreateInfo grid_vertex_input = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.vertexAttributeDescriptionCount = grid_attr_count,
		.pVertexBindingDescriptions = &grid_binding_desc,
		.pVertexAttributeDescriptions = grid_attr_descs,
	};

	VkPipelineInputAssemblyStateCreateInfo input_asm = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	VkPipelineInputAssemblyStateCreateInfo input_asm_grid = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	};

	VkViewport viewport = {
		.width = p_app->swp.render_extent.width,
		.height = p_app->swp.render_extent.height,
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
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.lineWidth = 1.0f,

		.depthBiasEnable = VK_TRUE,
    .depthBiasConstantFactor = 1.0f,
    .depthBiasClamp = 0.0f,
    .depthBiasSlopeFactor = 1.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisample = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = p_app->device.msaa_samples,
	};

	VkPipelineDepthStencilStateCreateInfo depth = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
	};

	VkPipelineColorBlendAttachmentState blend_opaque = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_FALSE
	};

	VkPipelineColorBlendAttachmentState blend_transparent = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
	};

	VkPipelineColorBlendAttachmentState blend_billboard = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
	};

	VkPipelineColorBlendAttachmentState blend_grid = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
	};

	VkPipelineColorBlendStateCreateInfo blend_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
	};

	VkPushConstantRange push_constant_range = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .offset = 0,
    .size = sizeof(_push_constants),
	};

	VkPipelineLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &p_app->pipeline.descriptor_set_layout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &push_constant_range,
	};

	if (vkCreatePipelineLayout(p_app->device.logical, &layout_info, NULL, &p_app->pipeline.layout) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "pipeline layout => failed");
		exit(EXIT_FAILURE);
	}

	VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pInputAssemblyState = &input_asm,
		.pViewportState = &viewport_state,
		.pRasterizationState = &raster,
		.pMultisampleState = &multisample,
		.pDepthStencilState = &depth,
		.pDynamicState = &dynamic_state,
		.pColorBlendState = &blend_state,
		.layout = p_app->pipeline.layout,
		.renderPass = p_app->pipeline.render_pass,
	};

	depth.depthWriteEnable = VK_TRUE;
	blend_state.pAttachments = &blend_opaque;
	pipeline_info.pStages = mesh_shader_stages;
	pipeline_info.pVertexInputState = &mesh_vertex_input;
	if (vkCreateGraphicsPipelines(p_app->device.logical, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &p_app->pipeline.opaque) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "opaque pipeline => failed");
		exit(EXIT_FAILURE);
	}

	depth.depthWriteEnable = VK_FALSE;
	blend_state.pAttachments = &blend_transparent;
	pipeline_info.pStages = mesh_shader_stages;
	pipeline_info.pVertexInputState = &mesh_vertex_input;
	if (vkCreateGraphicsPipelines(p_app->device.logical, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &p_app->pipeline.transparent) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "transparent pipeline => failed");
		exit(EXIT_FAILURE);
	}

	depth.depthWriteEnable = VK_FALSE;
	blend_state.pAttachments = &blend_billboard;
	pipeline_info.pStages = billboard_shader_stages;
	pipeline_info.pVertexInputState = &billboard_vertex_input;
	if (vkCreateGraphicsPipelines(p_app->device.logical, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &p_app->pipeline.billboard) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "billboard pipeline => failed");
		exit(EXIT_FAILURE);
	}

	depth.depthWriteEnable = VK_TRUE;
	blend_state.pAttachments = &blend_grid;
	pipeline_info.pStages = grid_shader_stages;
	pipeline_info.pVertexInputState = &grid_vertex_input;
	pipeline_info.pInputAssemblyState = &input_asm_grid;
	raster.cullMode = VK_CULL_MODE_NONE;
	if (vkCreateGraphicsPipelines(p_app->device.logical, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &p_app->pipeline.grid) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "grid pipeline => failed");
		exit(EXIT_FAILURE);
	}

	vkDestroyShaderModule(p_app->device.logical, mesh_frag_shader_module, NULL);
	vkDestroyShaderModule(p_app->device.logical, mesh_vert_shader_module, NULL);
	vkDestroyShaderModule(p_app->device.logical, billboard_frag_shader_module, NULL);
	vkDestroyShaderModule(p_app->device.logical, billboard_vert_shader_module, NULL);
	vkDestroyShaderModule(p_app->device.logical, grid_frag_shader_module, NULL);
	vkDestroyShaderModule(p_app->device.logical, grid_vert_shader_module, NULL);
	free((void*)mesh_vert_shader_code);
	free((void*)mesh_frag_shader_code);
	free((void*)billboard_vert_shader_code);
	free((void*)billboard_frag_shader_code);
	free((void*)grid_vert_shader_code);
	free((void*)grid_frag_shader_code);
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
