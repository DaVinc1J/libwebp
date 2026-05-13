#include "headers/core.h"
#include "headers/validation.h"

void create_instance(_app *p_app) {
	if (enable_validation_layers && !check_validation_layer_support()) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"validation layers requested but no available"
		);
		exit(EXIT_FAILURE);
	}

	u32 glfw_extensions_count = 0;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

	u32 required_extensions_count = glfw_extensions_count + (enable_validation_layers ? 1 : 0);
#ifdef __APPLE__
	required_extensions_count += 1;
#endif

	const char *required_extensions[required_extensions_count];

	for (u32 i = 0; i < glfw_extensions_count; i++) {
		required_extensions[i] = glfw_extensions[i];
	}

	u32 ext_i = glfw_extensions_count;

#ifdef __APPLE__
	required_extensions[ext_i++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
#endif

	if (enable_validation_layers) {
		required_extensions[ext_i++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}

	u32 api_version = 0;
	vkEnumerateInstanceVersion(&api_version);

	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = p_app->config.win.title,
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = p_app->config.win.title,
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = api_version,
		.pNext = NULL,
	};

	VkInstanceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info,
		.enabledExtensionCount = required_extensions_count,
		.ppEnabledExtensionNames = required_extensions,
	};

#ifdef __APPLE__
	create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
	if (enable_validation_layers) {
		create_info.enabledLayerCount = validation_layer_count;
		create_info.ppEnabledLayerNames = validation_layers;
		populate_debug_messenger_create_info(&debug_create_info);
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;
	} else {
		create_info.enabledLayerCount = 0;
		create_info.pNext = NULL;
	}

	if (vkCreateInstance(&create_info, NULL, &p_app->inst.instance) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"instance => failed to create instance"
		);
		exit(EXIT_FAILURE);
	}

	u32 extensions_count = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &extensions_count, NULL);
	VkExtensionProperties extensions[extensions_count];
	vkEnumerateInstanceExtensionProperties(NULL, &extensions_count, extensions);

	if (!verify_extensions_support(extensions_count, extensions, glfw_extensions_count, glfw_extensions)) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"instance => extensions support verify failed"
		);
		exit(EXIT_FAILURE);
	}
}

void create_surface(_app *p_app) {
	if (glfwCreateWindowSurface(p_app->inst.instance, p_app->win.window, NULL, &p_app->swp.surface) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"surface => failed to create surface"
		);
		exit(EXIT_FAILURE);
	}
}

void pick_physical_device(_app *p_app) {
	p_app->device.physical = VK_NULL_HANDLE;

	uint32_t physical_device_count = 0;
	vkEnumeratePhysicalDevices(p_app->inst.instance, &physical_device_count, NULL);

	if (physical_device_count == 0) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => failed to find any devices"
		);
		exit(EXIT_FAILURE);
	}

	VkPhysicalDevice physical_devices[physical_device_count];
	vkEnumeratePhysicalDevices(p_app->inst.instance, &physical_device_count, physical_devices);

	if (physical_device_count == 1) {
		if (!is_physical_device_suitable(p_app, physical_devices[0])) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"physical device => only device is not suitable"
			);
			exit(EXIT_FAILURE);
		}

		p_app->device.physical = physical_devices[0];
		get_max_usable_sample_count(p_app->device.physical, &p_app->device.msaa_samples);
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
			"physical device => only device is suitable"
		);
		return;
	}

	_candidates candidates[physical_device_count];

	int max_score = 0;
	int max_index = -1;

	for (int i = 0; i < physical_device_count; i++) {
		if (is_physical_device_suitable(p_app, physical_devices[i])) {
			candidates[i].p_physical_device = &physical_devices[i];
			rate_physical_device_suitability(&candidates[i]);

			if (candidates[i].score > max_score) {
				max_score = candidates[i].score;
				max_index = i;
			}
		}
	}

	if (max_index != -1) {
		p_app->device.physical = *candidates[max_index].p_physical_device;
		get_max_usable_sample_count(p_app->device.physical, &p_app->device.msaa_samples);
	}

	if (p_app->device.physical == VK_NULL_HANDLE) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => failed to find a suitable device"
		);
		exit(EXIT_FAILURE);
	}
}

bool is_physical_device_extensions_supported(VkPhysicalDevice physical_device) {

	u32 extensions_count;
	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extensions_count, NULL);

	VkExtensionProperties available_extensions[extensions_count];
	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extensions_count, available_extensions);

	for (u32 i = 0; i < enabled_logical_device_extensions_count; i++) {
		bool extension_found = false;
		for (u32 j = 0; j < extensions_count; j++) {
			if (strcmp(enabled_logical_device_extensions[i], available_extensions[j].extensionName) == 0) {
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

void create_logical_device(_app *p_app) {
	p_app->device.queue_indices = find_queue_families(p_app, p_app->device.physical);

	u32 unique_queue_families[2] = {
		p_app->device.queue_indices.graphics_family,
		p_app->device.queue_indices.present_family,
	};

	bool same_family = (unique_queue_families[0] == unique_queue_families[1]);

	VkDeviceQueueCreateInfo queue_create_infos[2];
	u32 queue_create_info_count = same_family ? 1 : 2;

	float queue_priority = 1.0f;

	queue_create_infos[0] = (VkDeviceQueueCreateInfo){
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = unique_queue_families[0],
		.queueCount = 1,
		.pQueuePriorities = &queue_priority,
	};

	if (!same_family) {
		queue_create_infos[1] = (VkDeviceQueueCreateInfo){
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = unique_queue_families[1],
			.queueCount = 1,
			.pQueuePriorities = &queue_priority
		};
	}

	VkPhysicalDeviceFeatures physical_device_features;
	vkGetPhysicalDeviceFeatures(p_app->device.physical, &physical_device_features);
	physical_device_features.samplerAnisotropy = VK_TRUE;
	physical_device_features.robustBufferAccess = VK_FALSE;

	VkPhysicalDeviceVulkan12Features features_1_2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.runtimeDescriptorArray = VK_TRUE,
		.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
	};

	VkDeviceCreateInfo logical_device_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = queue_create_info_count,
		.pQueueCreateInfos = queue_create_infos,
		.pEnabledFeatures = &physical_device_features,
		.enabledExtensionCount = enabled_logical_device_extensions_count,
		.ppEnabledExtensionNames = enabled_logical_device_extensions,
		.pNext = &features_1_2,
	};

	logical_device_create_info.enabledLayerCount = 0;

	if (enable_validation_layers) {
		logical_device_create_info.enabledLayerCount = validation_layer_count;
		logical_device_create_info.ppEnabledLayerNames = validation_layers;
	}

	if (vkCreateDevice(p_app->device.physical, &logical_device_create_info, NULL, &p_app->device.logical) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"logical device => failed to create logical device"
		);
		exit(EXIT_FAILURE);
	}

	vkGetDeviceQueue(p_app->device.logical, p_app->device.queue_indices.graphics_family, 0, &p_app->device.graphics_queue);
	vkGetDeviceQueue(p_app->device.logical, p_app->device.queue_indices.present_family, 0, &p_app->device.present_queue);
}
void create_alloc(_app *p_app) {

	VmaVulkanFunctions vulkan_funcs = {
		.vkGetInstanceProcAddr = &vkGetInstanceProcAddr,
		.vkGetDeviceProcAddr = &vkGetDeviceProcAddr,
	};

	u32 api_version = 0;
	vkEnumerateInstanceVersion(&api_version);

	VmaAllocatorCreateInfo allocatorCreateInfo = {
		.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
		.vulkanApiVersion = api_version,
		.physicalDevice = p_app->device.physical,
		.device = p_app->device.logical,
		.instance = p_app->inst.instance,
		.pVulkanFunctions = &vulkan_funcs,
	};

	if (vmaCreateAllocator(&allocatorCreateInfo, &p_app->mem.alloc) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance, 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"allocator => failed to create allocator"
		);
		exit(EXIT_FAILURE);
	}
}

void rate_physical_device_suitability(_candidates *p_candidate) {
	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceFeatures physical_device_features;
	vkGetPhysicalDeviceProperties(*p_candidate->p_physical_device, &physical_device_properties);
	vkGetPhysicalDeviceFeatures(*p_candidate->p_physical_device, &physical_device_features);

	p_candidate->score = 0;

	if (physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		p_candidate->score += 1000;
	}

	p_candidate->score += physical_device_properties.limits.maxImageDimension2D;

	if (physical_device_features.geometryShader) {
		p_candidate->score += 200;
	}

	// in the future to improve performance we should look for queue 
	// families that have both graphics and present support
	// as to not have them split, but atp it dont matter
}

_queue_family_indices find_queue_families(_app *p_app, VkPhysicalDevice physical_device) {
	_queue_family_indices indices;
	indices.is_graphics_family_set = 0;

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

	VkQueueFamilyProperties queue_family_properties[queue_family_count];
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties);

	VkBool32 present_support = false;
	for (int i = 0; i < queue_family_count; i++) {
		if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
			indices.is_graphics_family_set = 1;
		}
		present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, p_app->swp.surface, &present_support);
		if (present_support) {
			indices.present_family = i;
			indices.is_present_family_set = 1;
		}
	}

	return indices;
}

bool is_physical_device_suitable(_app *p_app, VkPhysicalDevice physical_device) {
	VkPhysicalDeviceFeatures physical_device_features;
	vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

	if (!physical_device_features.geometryShader) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
			"physical device => lacks geometry shader support"
		);
	}

	if (!physical_device_features.samplerAnisotropy) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => lacks sampler anisotropy"
		);
		return false;
	}

	_queue_family_indices indices = find_queue_families(p_app, physical_device);
	if (indices.is_graphics_family_set == 0 || indices.is_present_family_set == 0) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => failed to find queue families"
		);
		return false;
	}

	if (!is_physical_device_extensions_supported(physical_device)) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => extensions not supported"
		);
		return false;
	}

	_swapchain_support support;
	if (!check_swapchain_support(p_app, physical_device, &support)) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => swapchain not supported"
		);
		return false;
	}
	free_swapchain_support(&support);

	return true;
}

void get_max_usable_sample_count(VkPhysicalDevice physical_device, VkSampleCountFlagBits *p_msaa_samples) {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physical_device, &properties);

	VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts &
		properties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT) *p_msaa_samples = VK_SAMPLE_COUNT_64_BIT;
	else if (counts & VK_SAMPLE_COUNT_32_BIT) *p_msaa_samples = VK_SAMPLE_COUNT_32_BIT;
	else if (counts & VK_SAMPLE_COUNT_16_BIT) *p_msaa_samples = VK_SAMPLE_COUNT_16_BIT;
	else if (counts & VK_SAMPLE_COUNT_8_BIT)  *p_msaa_samples = VK_SAMPLE_COUNT_8_BIT;
	else if (counts & VK_SAMPLE_COUNT_4_BIT)  *p_msaa_samples = VK_SAMPLE_COUNT_4_BIT;
	else if (counts & VK_SAMPLE_COUNT_2_BIT)  *p_msaa_samples = VK_SAMPLE_COUNT_2_BIT;
	else *p_msaa_samples = VK_SAMPLE_COUNT_1_BIT;
}

void free_swapchain_support(_swapchain_support *p_support) {
	free(p_support->surface_formats);
	p_support->surface_formats = NULL;
	free(p_support->present_modes);
	p_support->present_modes = NULL;
}

bool check_swapchain_support(_app *p_app, VkPhysicalDevice physical_device, _swapchain_support *p_support) {

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, p_app->swp.surface, &p_support->capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, p_app->swp.surface, &p_support->surface_formats_count, NULL);
	if (p_support->surface_formats_count != 0) {
		p_support->surface_formats = malloc(sizeof(VkSurfaceFormatKHR) * p_support->surface_formats_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, p_app->swp.surface, &p_support->surface_formats_count, p_support->surface_formats);
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, p_app->swp.surface, &p_support->present_modes_count, NULL);
	if (p_support->present_modes_count != 0) {
		p_support->present_modes = malloc(sizeof(VkPresentModeKHR) * p_support->present_modes_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, p_app->swp.surface, &p_support->present_modes_count, p_support->present_modes);
	}

	return (p_support->surface_formats_count != 0 && p_support->present_modes_count != 0);
}
