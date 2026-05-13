#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;

typedef struct _app_webp {
	u8* data;
	size_t data_size;
	char* file;
} _app_webp;

typedef struct _app_output {
	u8* rgba;
} _app_output;

typedef struct _app {
	_app_webp webp;
	_app_output output;
} _app;

