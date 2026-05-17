#include "headers/define.h"
#include "headers/validation.h"
#include "headers/helper.h"
#include "headers/png.h"
#include "headers/jpg.h"
#include "headers/file.h"

void file_setup(_app *p_app, const char *filename) {
	DIR *p_dir = opendir("src/file");
	if (!p_dir) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"file read => could not open src/file/"
		);
		exit(1);
	}
	struct dirent *entry;
	int found = 0;
	while ((entry = readdir(p_dir)) != NULL) {
		if (strcmp(entry->d_name, filename) != 0) continue;
		char tmp[512];
		snprintf(tmp, sizeof(tmp), "src/file/%s", entry->d_name);
		p_app->file.path = strdup(tmp);
		found = 1;
		break;
	}
	closedir(p_dir);
	if (!found) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"file read => specified file not found in src/file/"
		);
		exit(1);
	}
}

void file_read(_app *p_app) {
	if (p_app->file.path == NULL) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"file read => no file loaded"
		);
		exit(1);
	}
	FILE *p_f = fopen(p_app->file.path, "rb");
	fseek(p_f, 0, SEEK_END);
	p_app->file.data_size = ftell(p_f);
	rewind(p_f);
	p_app->file.data = malloc(p_app->file.data_size);
	fread(p_app->file.data, 1, p_app->file.data_size, p_f);
	fclose(p_f);

	bytes p = p_app->file.data;
	usize size = p_app->file.data_size;

	p_app->file.type = _FILE_TYPE_NONE;
	if (copy_u64_be(p) == PNG_SIG) 		
		p_app->file.type = _FILE_TYPE_PNG;

	if (copy_u16_be(p) == JPG_SOI) 		
		p_app->file.type = _FILE_TYPE_JPG;

	if (copy_u32_be(p) == WEBP_RIFF_SIG && copy_u32_be(p + 8) == WEBP_SIG)
    p_app->file.type = _FILE_TYPE_WEBP;

	if (p_app->file.type == _FILE_TYPE_NONE) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"file read => unsupported file type (expected png/jpg/webp)"
		);
		exit(1);
	}
}

static void read_webp(_app *p_app) {
	printf("webp\n");
}

void file_decode(_app *p_app) {
	switch (p_app->file.type) {
		case _FILE_TYPE_PNG:  
			png(p_app);
			break;
		case _FILE_TYPE_JPG: 
			jpg(p_app); 
			break;
		case _FILE_TYPE_WEBP: 
			read_webp(p_app); 
			break;
	}
}
