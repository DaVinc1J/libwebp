#ifndef VK_BUFFER_H
#define VK_BUFFER_H

#include "define.h"

void create_buffer(_app *p_app, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, VkBuffer *p_buffer, VmaAllocation *p_allocation);

void create_command_pool(_app *p_app);
void create_command_buffers(_app *p_app);
void record_command_buffer(_app *p_app, VkCommandBuffer command_buffer, uint32_t image_index);

VkCommandBuffer begin_single_time_commands(_app *p_app);
void end_single_time_commands(_app *p_app, VkCommandBuffer command_buffer);

#endif
