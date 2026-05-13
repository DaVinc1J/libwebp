#ifndef SHADER_H
#define SHADER_H

#include "define.h"

VkVertexInputBindingDescription get_mesh_binding_description();
VkVertexInputBindingDescription get_billboard_binding_description();
VkVertexInputBindingDescription get_grid_binding_description();

void get_mesh_attribute_descriptions(VkVertexInputAttributeDescription* attribs, u32 *num_attribs);
void get_billboard_attribute_descriptions(VkVertexInputAttributeDescription* attribs, u32 *num_attribs);
void get_grid_attribute_descriptions(VkVertexInputAttributeDescription* attribs, u32 *num_attribs);

#endif
