#ifndef DEFINE_H
#define DEFINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <dirent.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <cglm/cglm.h>

#include <zlib.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "../libraries/vk_mem_alloc.h"
#ifdef __cplusplus
}
#endif

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#endif

#define PNG_SIG 0x89504e470d0a1a0a
#define PNG_CHUNK_IHDR ('I'<<24 | 'H'<<16 | 'D'<<8 | 'R')
#define PNG_CHUNK_IDAT ('I'<<24 | 'D'<<16 | 'A'<<8 | 'T')
#define PNG_CHUNK_IEND ('I'<<24 | 'E'<<16 | 'N'<<8 | 'D')
#define PNG_CHUNK_PLTE ('P'<<24 | 'L'<<16 | 'T'<<8 | 'E')
#define PNG_CHUNK_tRNS ('t'<<24 | 'R'<<16 | 'N'<<8 | 'S')
#define PNG_CHUNK_gAMA ('g'<<24 | 'A'<<16 | 'M'<<8 | 'A')

#define JPG_SOI   0xffd8
#define JPG_APP_0 0xffe0
#define JPG_COM   0xfffe
#define JPG_DQT   0xffdb
#define JPG_SOF_0 0xffc0
#define JPG_DHT   0xffc4
#define JPG_SOS   0xffda
#define JPG_EOI   0xffd9

#define WEBP_RIFF_SIG 0x52494646
#define WEBP_SIG 0x57454250

#define TITLE_PX_MIN 55.0f

#define true  1
#define false 0

#define DEFINE_HT_ID_TYPE(type_name, T) \
typedef union type_name {               \
	T raw[2];      	  									  \
	struct {             									\
		T dc;  		 													\
		T ac;	  	 													\
	};      								              \
} type_name;

#define DEFINE_HT_PAIR_TYPE(type_name, T) \
typedef union type_name {                 \
	T raw[2][2];                            \
	struct { T dc[2]; T ac[2]; };           \
	struct {                                \
		T dc_y; T dc_cbcr;                  	\
		T ac_y; T ac_cbcr;                  	\
	};                                      \
} type_name;

#define DEFINE_HT_NODES_TYPE(type_name, T) \
typedef union type_name {                  \
	T *raw[2][2];                            \
	struct { T *dc[2]; T *ac[2]; };          \
	struct {                                 \
		T *dc_y;                               \
		T *dc_cbcr;                         	 \
		T *ac_y;                             	 \
		T *ac_cbcr;                          	 \
	};                                       \
} type_name;

typedef uint8_t 	u8;
typedef uint16_t 	u16;
typedef uint32_t 	u32;
typedef uint64_t 	u64;
typedef int8_t 		i8;
typedef int16_t 	i16;
typedef int32_t 	i32;
typedef int64_t 	i64;
typedef size_t 		usize;
typedef ptrdiff_t isize;
typedef float 		f32;
typedef double 		f64;
typedef uint8_t 	bool8;
typedef void*     rawptr;
typedef const u8* bytes;

typedef enum _FILE_TYPE {
	_FILE_TYPE_NONE,
	_FILE_TYPE_PNG,
	_FILE_TYPE_JPG,
	_FILE_TYPE_WEBP,
	_FILE_TYPE_COUNT,
} _FILE_TYPE;

extern const u32 MAX_FRAMES_IN_FLIGHT;

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

typedef struct _png_info {
	u8 bit_depth;
	u8 colour_type;
	bool8 interlace;
	u8 channels;
	u32 stride;

	u8 palette[256 * 3];
	u32 palette_size;

	bool8 has_tRNS;
	u16 tRNS_gray;
	u16 tRNS_rgb[3];
	u8 tRNS_alpha[256];

	bool8 has_gAMA;
	f32 gamma;

	u8 *idat_buf;
	usize idat_size;
	usize idat_cap;

	u8 *out;
	usize out_size;
} _png_info;

typedef struct _huffman_table {
	i32  count[16];
	u8   symbols[256];
	i32  symbol_count;
} _huffman_table;

typedef struct _huffman_table_node {
	bool8  leaf;
	char   code[17];         // huffman code as bit string (max 16 bits + null)
	u16    value;
	i32    left;             // index into ht_nodes[], -1 = none
	i32    right;
	i32    parent;
} _huffman_table_node;

DEFINE_HT_ID_TYPE(_ht_id, u8)
typedef struct _component {
	u8 id;
	u8 h_samp;
	u8 v_samp;
	u8 qt_id;
	_ht_id ht_id; 
} _component;

DEFINE_HT_PAIR_TYPE(_ht_table, _huffman_table)
DEFINE_HT_PAIR_TYPE(_ht_set, bool8)
DEFINE_HT_PAIR_TYPE(_ht_root, i32)
DEFINE_HT_PAIR_TYPE(_ht_node_count, i32)
DEFINE_HT_NODES_TYPE(_ht_nodes, _huffman_table_node)
typedef struct _jpg_info {
	u8 comp_count;
	_component comp[3];

	u16 qt[4][64];
	bool8 qt_set[4];

	_ht_table ht;
	_ht_set ht_set;
	_ht_root ht_root;
	_ht_node_count ht_node_count;
	_ht_nodes ht_nodes;

	bytes scan_data;
	usize scan_size;

	i32 dc_pred[3];

	i32 (*mcu_data)[3][64];
	i32 mcu_count;

	u8 *out;
	usize out_size;
} _jpg_info;

typedef struct _pc {
	vec2 uv_scale;
	vec2 border_uv;
	vec4 border_colour;
	vec4 background_colour;
	vec4 title_colour;
	f32 title_uv;
} _pc;

typedef struct _candidates {
	VkPhysicalDevice *p_physical_device;
	u32 score;
} _candidates;

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
} _app_device;

typedef struct _app_swapchain {
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkImage* images;
	u32 images_count;
	VkImageView* image_views;
	VkSurfaceFormatKHR surface_format;
	VkExtent2D extent;
	bool8 framebuffer_resized;
} _app_swapchain;

typedef struct _app_pipeline {
	VkRenderPass render_pass;
	VkDescriptorSetLayout descriptor_set_layout;
	VkPipelineLayout layout;
	VkPipeline pipeline;
	VkFramebuffer* swapchain_framebuffers;
} _app_pipeline;

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

typedef struct _app_descriptors {
	VkDescriptorPool pool;
	VkDescriptorSet* sets;
} _app_descriptors;

typedef struct _app_image {
	u32 width;
	u32 height;

	VkImage image;
	VmaAllocation image_allocation;
	VkImageView image_view;
	VkSampler sampler;

	VkBuffer staging_buffer;
	VmaAllocation staging_allocation;
	void* staging_mapped;
	VkDeviceSize staging_size;

	bool8 created;
} _app_image;

typedef struct _app_config {
	char *title;
	u32 width;
	u32 height;
	char *vert_shader;
	char *frag_shader;
	bool8 print_fps;
	f32 border_px;
	f32 title_px;
	vec4 border_colour;
	vec4 background_colour;
	vec4 title_colour;
	bool8 border;
	bool8 blur;
} _app_config;

typedef struct _app_performance {
	struct timespec last_frame_time;
	float frame_time_avg;
	float fps_avg;
	int frame_count;
} _app_performance;

typedef struct _app_file {
	u8 type;
	size_t data_size;
	u8* data;
	char* path;
	rawptr info;
} _app_file;

typedef struct _app_output {
	u8* pixels;
	u32 width;
	u32 height;
} _app_output;

typedef struct _app {
	_app_window win;
	_app_instance inst;
	_app_device device;
	_app_swapchain swp;
	_app_pipeline pipeline;
	_app_commands cmd;
	_app_sync sync;
	_app_memory mem;
	_app_descriptors descriptor;
	_app_image image;
	_app_config config;
	_app_performance perf;

	_app_file file;
	_app_output output;
} _app;

#endif
