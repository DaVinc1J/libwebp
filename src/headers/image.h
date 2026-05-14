#ifndef VK_IMAGE_H
#define VK_IMAGE_H

#include "define.h"

void create_image(_app *p_app, VkImage *p_image, VmaAllocation *p_allocation, u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memory_usage);
void transition_image_layout(_app *p_app, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);
void copy_buffer_to_image(_app *p_app, VkBuffer buffer, VkImage image, u32 width, u32 height);

void create_image_resources(_app *p_app, u32 width, u32 height);
void destroy_image_resources(_app *p_app);

void set_image_data(_app *p_app);

#endif
