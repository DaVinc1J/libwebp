#ifndef VK_PIPELINE_H
#define VK_PIPELINE_H

#include "define.h"

char* read_file(_app *p_app, const char *filename, size_t* shader_code_size);
void create_graphics_pipelines(_app *p_app);
VkShaderModule create_shader_module(_app *p_app, const char* shader_code, size_t shader_code_size);

#endif
