#ifndef VK_CORE_H
#define VK_CORE_H

#include "define.h"

void create_instance(_app *p_app);
void create_surface(_app *p_app);
void pick_physical_device(_app *p_app);
void create_logical_device(_app *p_app);
void create_alloc(_app *p_app);

bool is_physical_device_extensions_supported(VkPhysicalDevice physical_device);
void rate_physical_device_suitability(_candidates *p_candidate);
_queue_family_indices find_queue_families(_app *p_app, VkPhysicalDevice physical_device);
bool is_physical_device_suitable(_app *p_app, VkPhysicalDevice physical_device);
void get_max_usable_sample_count(VkPhysicalDevice physical_device, VkSampleCountFlagBits *p_msaa_samples);
void free_swapchain_support(_swapchain_support *p_support);
bool check_swapchain_support(_app *p_app, VkPhysicalDevice physical_device, _swapchain_support *p_support);

#endif
