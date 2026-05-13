#ifndef VALIDATION_H
#define VALIDATION_H

#include "define.h"

extern const bool enable_validation_layers;
extern const char* enabled_logical_device_extensions[];
extern const u32 enabled_logical_device_extensions_count;
extern const char* validation_layers[];
extern const u32 validation_layer_count;

bool verify_extensions_support(u32 extensions_count, VkExtensionProperties *extensions, u32 glfw_extensions_count, const char **glfw_extensions);
bool check_validation_layer_support();

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data);
VkResult create_debug_utils_messenger_EXT(VkInstance inst, const VkDebugUtilsMessengerCreateInfoEXT *p_create_info, const VkAllocationCallbacks *p_alloc, VkDebugUtilsMessengerEXT *p_debug_messenger);
void destroy_debug_utils_messenger_EXT(VkInstance inst, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks *p_alloc);
void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info);
void setup_debug_messenger(_app *p_app);
void submit_debug_message(VkInstance inst, VkDebugUtilsMessageSeverityFlagBitsEXT severity, const char* fmt, ...);

#endif
