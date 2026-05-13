#ifndef VK_SWAPCHAIN_H
#define VK_SWAPCHAIN_H

#include "define.h"

VkSurfaceFormatKHR choose_swapchain_surface_format(_app *p_app, _swapchain_support *p_support);
VkPresentModeKHR choose_swapchain_present_mode(_app *p_app, _swapchain_support *p_support);
VkExtent2D choose_swapchain_swap_extent(_app *p_app, _swapchain_support *p_support);

void create_swapchain(_app *p_app);

void create_image_view(_app *p_app, VkImage image, VkImageView *p_image_view, u32 mip_levels, VkFormat format, VkImageAspectFlags aspect_flags);
void create_image_views(_app *p_app);

void create_framebuffers(_app *p_app);

void cleanup_swapchain(_app *p_app);
void recreate_swapchain(_app *p_app);

#endif
