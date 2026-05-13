#ifndef VK_IMAGE_H
#define VK_IMAGE_H

#include "define.h"


VkFormat find_supported_format(_app *p_app, VkPhysicalDevice physical_device, const VkFormat *candidates, size_t candidate_count, VkImageTiling tiling, VkFormatFeatureFlags features);
VkFormat find_depth_format(_app *p_app, VkPhysicalDevice physical_device);
bool has_stencil_component(VkFormat format);

void create_colour_resources(_app *p_app);
void create_depth_resources(_app *p_app);
void create_resolve_resources(_app *p_app);

void create_mipmaps(_app *p_app, VkImage image, u32 mip_levels, i32 tex_width, i32 tex_height, VkFormat image_format);
void create_image(_app *p_app, VkImage *p_image, u32 mip_levels, VkSampleCountFlagBits num_samples, VmaAllocation *p_allocation, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memory_usage);
void transition_image_layout(_app *p_app, VkImage image, u32 mip_levels, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
void copy_buffer_to_image(_app *p_app, VkBuffer buffer, VkImage image, u32 width, u32 height);

#endif
