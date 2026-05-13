#include "headers/validation.h"

#ifdef NDEBUG
const bool enable_validation_layers = false;
#else
const bool enable_validation_layers = true;
#endif

#ifdef __APPLE__
const char* enabled_logical_device_extensions[] = {
    "VK_KHR_swapchain",
    "VK_KHR_portability_subset"
};
#else
const char* enabled_logical_device_extensions[] = {
    "VK_KHR_swapchain"
};
#endif

const u32 enabled_logical_device_extensions_count =
    sizeof(enabled_logical_device_extensions) / sizeof(enabled_logical_device_extensions[0]);

const char* validation_layers[] = { "VK_LAYER_KHRONOS_validation" };
const u32 validation_layer_count =
    sizeof(validation_layers) / sizeof(validation_layers[0]);

bool verify_extensions_support(u32 extensions_count, VkExtensionProperties *extensions, u32 glfw_extensions_count, const char **glfw_extensions) {

	for (int i = 0; i < glfw_extensions_count; i++) {
		bool extension_found = false;
		for (int j = 0; j < extensions_count; j++) {
			if (strcmp(extensions[j].extensionName, glfw_extensions[i]) == 0) {
				extension_found = true;
				break;
			}
		}
		if (!extension_found) {
			return false;
		}
	}

	return true;
}

bool check_validation_layer_support() {
	u32 layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);
	VkLayerProperties available_layers[layer_count];
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

	bool layer_found = false;
	for (u32 i = 0; i < validation_layer_count; i++) {
		layer_found = false;
		for (u32 j = 0;j < layer_count; j++) {
			if (strcmp(available_layers[j].layerName, validation_layers[i]) == 0) {
				layer_found = true;
				break;
			}
		}
		if (!layer_found) {
			return false;
		}
	}
	return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_type,
	const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
	void* p_user_data) {

	if (strstr(p_callback_data->pMessage, "VK_KHR_portability_subset") != NULL || strstr(p_callback_data->pMessage, "vkGetPhysicalDeviceProperties2KHR") != NULL) {
		return VK_FALSE;
	}

	fprintf(stderr, "validation layer: %s\n", p_callback_data->pMessage);

	return VK_FALSE;
}

VkResult create_debug_utils_messenger_EXT(VkInstance inst, const VkDebugUtilsMessengerCreateInfoEXT *p_create_info, const VkAllocationCallbacks *p_alloc, VkDebugUtilsMessengerEXT *p_debug_messenger) {
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(inst, "vkCreateDebugUtilsMessengerEXT");
	if (func != NULL) {
		return func(inst, p_create_info, p_alloc, p_debug_messenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
void destroy_debug_utils_messenger_EXT(VkInstance inst, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks *p_alloc) {
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(inst, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL) {
		func(inst, debug_messenger, p_alloc);
	}
}

void setup_debug_messenger(_app *p_app) {
	if (!enable_validation_layers) return;

	VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
	populate_debug_messenger_create_info(&create_info);

	if (create_debug_utils_messenger_EXT(p_app->inst.instance, &create_info, NULL, &p_app->inst.debug_messenger) != VK_SUCCESS) {
		printf("failed to setup debug messenger\n");
		exit(EXIT_FAILURE);
	}
}

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info) {
	create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info->pfnUserCallback = debug_callback;
};

void submit_debug_message(VkInstance inst, VkDebugUtilsMessageSeverityFlagBitsEXT severity, const char* fmt, ...) {
	if (!enable_validation_layers) return;

	char buffer[1024];

	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	PFN_vkSubmitDebugUtilsMessageEXT func = 
		(PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(inst, "vkSubmitDebugUtilsMessageEXT");

	if (func != NULL) {
		func(inst,
			 severity,
			 VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
			 &(VkDebugUtilsMessengerCallbackDataEXT){
			 .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT,
			 .pMessage = buffer
			 });
	}
}
