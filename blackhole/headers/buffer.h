#ifndef VK_BUFFER_H
#define VK_BUFFER_H

#include "define.h"

void create_buffer(_app *p_app, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, VkBuffer *p_buffer, VmaAllocation *p_allocation);
void copy_buffer(_app *p_app, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
void create_billboard_buffer(_app *p_app);
void create_mesh_buffer(_app *p_app);
void create_grid_buffer(_app *p_app);
void create_uniform_buffers(_app *p_app);
void create_storage_buffers(_app *p_app);
void recreate_billboard_storage_buffers(_app *p_app, VkDeviceSize new_size);
void recreate_solar_object_storage_buffers(_app *p_app, VkDeviceSize new_size);

void create_command_pool(_app *p_app);
void create_command_buffers(_app *p_app);
int compare_render_order(const void* a, const void* b);
void record_command_buffer(_app *p_app, VkCommandBuffer command_buffer, uint32_t image_index);
VkCommandBuffer begin_single_time_commands(_app *p_app);
void end_single_time_commands(_app *p_app, VkCommandBuffer command_buffer);

#endif
