#include "headers/shader.h"

const u32 number_of_mesh_attributes = 4;
const u32 number_of_billboard_attributes = 4;
const u32 number_of_grid_attributes = 1;

VkVertexInputBindingDescription get_mesh_binding_description() {
	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = 0;
	binding_description.stride = sizeof(_vertex);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_description;
}

VkVertexInputBindingDescription get_billboard_binding_description() {
	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = 0;
	binding_description.stride = sizeof(_billboard);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	return binding_description;
}

VkVertexInputBindingDescription get_grid_binding_description() {
	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = 0;
	binding_description.stride = sizeof(_grid_vertex);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_description;
}

void get_mesh_attribute_descriptions(VkVertexInputAttributeDescription* attribs, u32 *num_attribs) {
	if (attribs == NULL) {
		*num_attribs = number_of_mesh_attributes;
		return;
	}

	attribs[0].binding = 0;
	attribs[0].location = 0;
	attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribs[0].offset = offsetof(_vertex, pos);

	attribs[1].binding = 0;
	attribs[1].location = 1;
	attribs[1].format = VK_FORMAT_R32G32_SFLOAT;
	attribs[1].offset = offsetof(_vertex, uv);

	attribs[2].binding = 0;
	attribs[2].location = 2;
	attribs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribs[2].offset = offsetof(_vertex, norm);

	attribs[3].binding = 0;
	attribs[3].location = 3;
	attribs[3].format = VK_FORMAT_R32G32B32A32_UINT;
	attribs[3].offset = offsetof(_vertex, data);
}

void get_billboard_attribute_descriptions(VkVertexInputAttributeDescription* attribs, u32 *num_attribs) {
	if (attribs == NULL) {
		*num_attribs = number_of_billboard_attributes;
		return;
	}

	attribs[0].binding = 0;
	attribs[0].location = 0;
	attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attribs[0].offset = offsetof(_billboard, pos_w);

	attribs[1].binding = 0;
	attribs[1].location = 1;
	attribs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attribs[1].offset = offsetof(_billboard, size_rotation);

	attribs[2].binding = 0;
	attribs[2].location = 2;
	attribs[2].format = VK_FORMAT_R32G32B32A32_UINT;
	attribs[2].offset = offsetof(_billboard, type_data);

	attribs[3].binding = 0;
	attribs[3].location = 3;
	attribs[3].format = VK_FORMAT_R32G32B32A32_UINT;
	attribs[3].offset = offsetof(_billboard, flags);
}


void get_grid_attribute_descriptions(VkVertexInputAttributeDescription* attribs, u32 *num_attribs) {
	if (attribs == NULL) {
		*num_attribs = number_of_grid_attributes;
		return;
	}

	attribs[0].binding = 0;
	attribs[0].location = 0;
	attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribs[0].offset = offsetof(_grid_vertex, pos);
}
