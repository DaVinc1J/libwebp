#ifndef DEFINE_H
#define DEFINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <cglm/cglm.h>
#include <freetype/freetype.h>
#include "../libraries/fast_obj.h"
#include "../libraries/cgltf.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../libraries/vk_mem_alloc.h"
#include "../libraries/stb_image.h"
#include "../libraries/stb_image_write.h"
#ifdef __cplusplus
}
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;

#define PLACEHOLDER_ZERO 0
#define MAX_ATLAS_FILES 128
#define MAX_OBJECT_FILES 16
#define MAX_LIGHTS 16
#define POSITION_SCALE 1.0f / 1.496e11f
#define VELOCITY_SCALE 1.0f / 29.78e3f
#define MASS_SCALE 1.0f / 1.989e30f
#define RADIUS_SCALE 1.0f / 6.957e8f     
#define SBO_HEADER_SIZE (sizeof(u32) * 4)
#define COLOUR_NOT_SET 0xFFFFFFFF

extern const u32 MAX_FRAMES_IN_FLIGHT;

u32 clamp(u32 n, u32 min, u32 max);

typedef enum _config_flags {
	CONFIG_FLAG_NONE = 0,
	CONFIG_FLAG_PRINT_FPS = 1 << 0,
} _config_flags;

typedef enum _sim_flags {
	SIM_FLAG_NONE = 0,
	SIM_FLAG_CALCULATE_GRAVITY = 1 << 0,
	SIM_FLAG_RENDER_GRID = 1 << 1,
} _sim_flags;

typedef enum _billboard_type_flags {
	BILLBOARD_TYPE_PLAIN = 0,
	BILLBOARD_TYPE_LIGHT = 1 << 0,
	BILLBOARD_TYPE_TEXTURE = 1 << 1,
} _billboard_type_flags;

typedef enum _billboard_shape_flags {
	BILLBOARD_SHAPE_CIRCLE = 0,
	BILLBOARD_SHAPE_SQUARE = 1 << 0,
} _billboard_shape_flags;

typedef enum _billboard_location_flags {
	BILLBOARD_LOCATION_IN_WORLD = 0,
	BILLBOARD_LOCATION_IN_HUD = 1 << 0,
} _billboard_location_flags;

typedef enum _mesh_sphere_lod {
	MESH_SPHERE_LOD0,
	MESH_SPHERE_LOD1,
	MESH_SPHERE_LOD2,
	MESH_SPHERE_LOD3,
	MESH_SPHERE_LOD_COUNT,
} _mesh_sphere_lod;

typedef enum _mesh_shape_type {
	MESH_SHAPE_SPHERE_LOD0,
	MESH_SHAPE_SPHERE_LOD1,
	MESH_SHAPE_SPHERE_LOD2,
	MESH_SHAPE_SPHERE_LOD3,
	MESH_SHAPE_COUNT,
} _mesh_shape_type;

typedef enum _solar_object_type {
	SOLAR_OBJECT_TYPE_PLAIN,
	SOLAR_OBJECT_TYPE_LIGHT_EMIT,
	SOLAR_OBJECT_TYPE_BILLBOARD,
	SOLAR_OBJECT_TYPE_BLACKHOLE,
	SOLAR_OBJECT_TYPE_COUNT,
} _solar_object_type;

typedef enum _planet_type {
    PLANET_TYPE_ROCKY,
    PLANET_TYPE_ICY,
    PLANET_TYPE_GAS_GIANT,
    PLANET_TYPE_ICE_GIANT,
    PLANET_TYPE_STAR,
    PLANET_TYPE_WHITE_DWARF,
    PLANET_TYPE_COUNT,
} _planet_type;

typedef struct _billboard_legacy {
	vec4 pos;
	vec4 data;
	u32 udata[4];
	u32 flags[4];
} _billboard_legacy;

typedef struct _push_constants {
	u32 object_index;
} _push_constants;

typedef struct _billboard {
	union {
		vec4 pos_w;
		struct {
			vec3 position;
			float intensity;
		} light_pos_w;
		struct {
			vec3 position;
			float alpha;
		} texture_pos_w;
		struct {
			vec3 position;
			float alpha;
		} plain_pos_w;
	};

	union {
		vec4 size_rotation;
		struct {
			vec2 size;
			vec2 rotation;
		};
	};

	union {
		u32 type_data[4];
		struct {
			u32 data0, data1, data2, data3;
		};
		struct {
			u32 colour_id;
			u32 reserved1;
			u32 reserved2;
			u32 reserved3;
		} light_data;
		struct {
			u32 texture_id;
			u32 reserved1;
			u32 reserved2;
			u32 reserved3;
		} texture_data;
		struct {
			u32 colour_id;
			u32 reserved1;
			u32 reserved2;
			u32 reserved3;
		} plain_data;
	};

	union {
		u32 flags[4];
		struct {
			u32 type, shape, location, reserved;
		};
	};
} _billboard;

typedef struct _vertex {
	float pos[3];
	float uv[2];
	float norm[3];
	u32 data[4];
} _vertex;

typedef struct _grid_vertex {
	float pos[3];
} _grid_vertex;

typedef struct _solar_object {
    vec3 position;     float _pad0; 
    vec3 velocity;     float _pad1;
    vec3 acceleration; float _pad2;
    float mass;
    float radius;
    u32 colour_id;
    u32 billboard_index;
    u32 type;
    u32 planet_type;
		float intensity;
		float schwarzschild_radius;
} _solar_object;

typedef enum _colour_hex {
    COLOUR_RED     = 0xFF0000,
    COLOUR_GREEN   = 0x00FF00, 
    COLOUR_BLUE    = 0x0000FF,
    COLOUR_YELLOW  = 0xFFFF00,
    COLOUR_CYAN    = 0x00FFFF,
    COLOUR_WHITE   = 0xFFFFFF,
    COLOUR_PURPLE  = 0x800080,
    COLOUR_ORANGE  = 0xFF8000,

    COLOUR_ROCKY      = 0x8B4513,
    COLOUR_ICY        = 0xB0E0E6,
    COLOUR_GAS_GIANT  = 0xD2A679,
    COLOUR_ICE_GIANT  = 0x4169E1,
    COLOUR_STAR       = 0xFFD27F,
    COLOUR_WHITE_DWARF= 0xE6F2FF,
} _colour_hex;

typedef struct _node {
	_vertex vertex;
	u32 index;
	struct _node* next;
} _node;

typedef struct _candidates {
	VkPhysicalDevice *p_physical_device;
	u32 score;
} _candidates;

typedef struct _ubo {
	mat4 proj;
	mat4 view;
  mat4 inv_proj;
  mat4 inv_view;
	vec4 ambient;
	vec4 grid_params;
} _ubo;

typedef struct _sbo {
	u32 billboard_count;
	_billboard *billboards;
} _sbo;

typedef struct _render_order {
	u32 object_index;
	float distance;
} _render_order;

typedef struct _queue_family_indices {
	u32 graphics_family;
	u32 present_family;
	u32 is_graphics_family_set;
	u32 is_present_family_set;
} _queue_family_indices;

typedef struct _swapchain_support {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR *surface_formats;
	VkPresentModeKHR *present_modes;
	u32 surface_formats_count;
	u32 present_modes_count;
} _swapchain_support;

typedef struct _app_window {
	GLFWwindow* window;
} _app_window;

typedef struct _app_instance {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
} _app_instance;

typedef struct _app_device {
	VkPhysicalDevice physical;
	VkDevice logical;
	VkQueue graphics_queue;
	VkQueue present_queue;
	_queue_family_indices queue_indices;
	VkSampleCountFlagBits msaa_samples;
} _app_device;

typedef struct _app_swapchain {
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkImage* images;
	u32 images_count;
	VkImageView* image_views;
	VkSurfaceFormatKHR surface_format;
	VkExtent2D extent;
	VkExtent2D render_extent;
	bool framebuffer_resized;
} _app_swapchain;

typedef struct _app_pipeline {
	VkRenderPass render_pass;
	VkDescriptorSetLayout descriptor_set_layout;
	VkPipelineLayout layout;
	VkPipeline opaque;
	VkPipeline transparent;
	VkPipeline billboard;
	VkPipeline grid;
	VkFramebuffer* swapchain_framebuffers;
} _app_pipeline;

typedef struct _app_lens {
	struct {
		VkRenderPass render_pass;
		VkFramebuffer framebuffer;
		VkDescriptorSetLayout descriptor_set_layout;
		VkPipelineLayout layout;
		VkPipeline pipeline;
	} pass;
	struct {
		VkDescriptorPool pool;
		VkDescriptorSet *sets;
	} descriptor;
	struct {
		VkImage image;
		VmaAllocation image_allocation;
		VkImageView image_view;
		VkSampler sampler;
	} target;
} _app_lens;

typedef struct _app_commands {
	VkCommandPool pool;
	VkCommandBuffer* buffers;
} _app_commands;

typedef struct _app_sync {
	VkSemaphore* image_available_semaphores;
	VkSemaphore* render_finished_semaphores;
	VkFence* in_flight_fences;
	u32 frame_index;
} _app_sync;

typedef struct _app_memory {
	VmaAllocator alloc;
} _app_memory;

typedef struct _app_mesh {
	_vertex** vertices; 
  u32** indices;
	VkBuffer* vertex_buffers;
	VkBuffer* index_buffers;
	VmaAllocation* vertex_allocations;
	VmaAllocation* index_allocations;
	u32* vertex_counts;
	u32* index_counts;
} _app_mesh;

typedef struct _app_billboard {
	VkBuffer instance_buffer;
	VmaAllocation instance_allocation;
	void* buffers_mapped;
	size_t current_buffer_size;
} _app_billboard;

typedef struct _app_grid {
	VkBuffer vertex_buffer;
	VmaAllocation vertex_allocation;
	u32 vertex_count;
	_grid_vertex *verts;
} _app_grid;

typedef struct _app_uniforms {
	VkBuffer* buffers;
	VmaAllocation* buffer_allocations;
	void** buffers_mapped;
} _app_uniforms;

typedef struct _app_storages {
	VkBuffer* billboard_buffers;
	VmaAllocation* billboard_buffer_allocations;
	void** billboard_buffers_mapped;
	size_t billboard_current_buffer_size;
	
	VkBuffer* solar_object_buffers;
	VmaAllocation* solar_object_buffer_allocations;
	void** solar_object_buffers_mapped;
	size_t solar_object_current_buffer_size;
} _app_storages;

typedef struct _app_descriptors {
	VkDescriptorPool pool;
	VkDescriptorSet* sets;
} _app_descriptors;

typedef struct _app_depth {
	VkImage image;
	VmaAllocation image_allocation;
	VkImageView image_view;
} _app_depth;

typedef struct _app_colour {
	VkImage image;
	VmaAllocation image_allocation;
	VkImageView image_view;
} _app_colour;

typedef struct _app_resolve {
	VkImage image;
	VmaAllocation image_allocation;
	VkImageView image_view;
} _app_resolve;

typedef struct _app_shader {
	char *mesh_vert;
	char *mesh_frag;
	char *billboard_vert;
	char *billboard_frag;
	char *grid_vert;
	char *grid_frag;
	char *lens_vert;
	char *lens_frag;
} _app_shader;

typedef struct _app_config {
	struct {
		char *title;
		u32 width;
		u32 height;
		u8 flags;
		float render_extent_modifier;
	} win;
	struct {
		u32 MESH_SPHERE_LOD_SEGMENTS[MESH_SPHERE_LOD_COUNT];
		u32 MESH_SPHERE_LOD_RINGS[MESH_SPHERE_LOD_COUNT];
		float MESH_SPHERE_LOD_DISTANCES[MESH_SPHERE_LOD_COUNT - 1];
		float MESH_SPHERE_LOD_RADIUS_MODIFIER;
	} lod;
	struct {
		float range;
		float spacing;
		float seg_len;
		float depth_multiplier;
		float softening_multiplier;
	} grid;
	struct {
		u32 flags;
	} sim;
} _app_config;

typedef struct _app_objects {
	_solar_object *solar_objects;
	u32 solar_object_count;
	_billboard *billboards;
	u32 billboard_count;
	u32 billboard_max;
	u32 primitive_count;
} _app_objects;

typedef struct _app_lighting {
	vec4 ambient;
} _app_lighting;

typedef struct _app_view {
	vec3 camera_pos;
	vec3 target;
	vec3 world_up;
	float fov_y;
	float near_plane;
	float far_plane;
	float rotation_speed;
	float sensitivity;
	float speed;
	float lerp_speed;
	vec3 cam_offset;
	vec3 cam_offset_goal;
	double last_mouse_x, last_mouse_y;
	bool first_mouse;
	bool mouse_locked;
	float yaw, pitch;
} _app_view;

typedef struct _app_performance {
	struct timespec last_frame_time;
	float delta_time;
	float frame_time_avg;
	float fps_avg;
	int frame_count;
} _app_performance;

typedef struct _app {
	_app_window win;
	_app_instance inst;
	_app_device device;
	_app_swapchain swp;
	_app_pipeline pipeline;
	_app_commands cmd;
	_app_sync sync;
	_app_memory mem;
	_app_uniforms uniform;
	_app_storages storage;
	_app_descriptors descriptor;
	_app_depth depth;
	_app_colour colour;
	_app_resolve resolve;
	_app_config config;
	_app_mesh mesh;
	_app_billboard billboard;
	_app_grid grid;
	_app_objects obj;
	_app_shader shader;
	_app_view view;
	_app_performance perf;
	_app_lighting lighting;
	_app_lens lens;
} _app;

#endif
