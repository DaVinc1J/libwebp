#include "headers/define.h"
#include "headers/validation.h"

static inline u64 read_u64_be(bytes *p_p) {
	u8 b0 = *(*p_p)++;
	u8 b1 = *(*p_p)++;
	u8 b2 = *(*p_p)++;
	u8 b3 = *(*p_p)++;
	u8 b4 = *(*p_p)++;
	u8 b5 = *(*p_p)++;
	u8 b6 = *(*p_p)++;
	u8 b7 = *(*p_p)++;
	return (u64)b0 << 56 | (u64)b1 << 48 | (u64)b2 << 40 | (u64)b3 << 32 | (u64)b4 << 24 | (u64)b5 << 16 | (u64)b6 << 8 | (u64)b7;
}

static inline u32 read_u32_be(bytes *p_p) {
	u8 b0 = *(*p_p)++;
	u8 b1 = *(*p_p)++;
	u8 b2 = *(*p_p)++;
	u8 b3 = *(*p_p)++;
	return (u32)b0 << 24 | (u32)b1 << 16 | (u32)b2 << 8 | (u32)b3;
}

static inline u16 read_u16_be(bytes *p_p) {
	u8 b0 = *(*p_p)++;
	u8 b1 = *(*p_p)++;
	return (u16)b0 << 8 | (u16)b1;
}

static inline u64 read_u64_le(bytes *p_p) {
	u8 b0 = *(*p_p)++;
	u8 b1 = *(*p_p)++;
	u8 b2 = *(*p_p)++;
	u8 b3 = *(*p_p)++;
	u8 b4 = *(*p_p)++;
	u8 b5 = *(*p_p)++;
	u8 b6 = *(*p_p)++;
	u8 b7 = *(*p_p)++;
	return (u64)b0 | (u64)b1 << 8 | (u64)b2 << 16 | (u64)b3 << 24 | (u64)b4 << 32 | (u64)b5 << 40 | (u64)b6 << 48 | (u64)b7 << 56;
}

static inline u32 read_u32_le(bytes *p_p) {
	u8 b0 = *(*p_p)++;
	u8 b1 = *(*p_p)++;
	u8 b2 = *(*p_p)++;
	u8 b3 = *(*p_p)++;
	return (u32)b0 | (u32)b1 << 8 | (u32)b2 << 16 | (u32)b3 << 24;
}

static inline u16 read_u16_le(bytes *p_p) {
	u8 b0 = *(*p_p)++;
	u8 b1 = *(*p_p)++;
	return (u16)b0 | (u16)b1;
}

static inline u8 read_u8(bytes *p_p) {
	return *(*p_p)++;
}

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

	const u8 *data = p_app->file.data;
	const usize size = p_app->file.data_size;

	static const u8 png[8]  = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};
	static const u8 jpg[3]  = {0xFF,0xD8,0xFF};
	static const u8 webp[4] = {0x52,0x49,0x46,0x46};

	p_app->file.type = _FILE_TYPE_NONE;
	if (size >= 8 && memcmp(data, png,  8) == 0) 		
		p_app->file.type = _FILE_TYPE_PNG;

	if (size >= 3 && memcmp(data, jpg,  3) == 0) 		
		p_app->file.type = _FILE_TYPE_JPG;

	if (size >= 12 && memcmp(data, webp, 4) == 0 && memcmp(data + 8, "WEBP", 4) == 0) 
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

void read_png_ihdr(_app *p_app, bytes *p_data, _png_info *p_info) {
	if (read_u32_be(p_data) != 13) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"png decode => invalid IHDR length"
		);
		exit(1);
	}

	if (read_u32_be(p_data) != PNG_CHUNK_IHDR) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"png decode => first chunk is not IHDR"
		);
		exit(1);
	}

	p_app->output.width = read_u32_be(p_data); 
	p_app->output.height = read_u32_be(p_data); 
	p_info->bit_depth = read_u8(p_data);
	p_info->colour_type = read_u8(p_data);
	if (read_u8(p_data) != 0) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"png decode => compression method is invalid"
		);
		exit(1);
	}
	if (read_u8(p_data) != 0) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"png decode => filter method is invalid"
		);
		exit(1);
	}
	p_info->interlace = read_u8(p_data);
	read_u32_be(p_data);

	switch (p_info->colour_type) {
		case 0: p_info->channels = 1; break;
		case 2: p_info->channels = 3; break;
		case 3: p_info->channels = 1; break;
		case 4: p_info->channels = 2; break;
		case 6: p_info->channels = 4; break;
		default: {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"png decode => invalid colour type"
			);
			exit(1);
		}
	}
	p_info->stride = p_app->output.width * p_info->channels;
}

static void read_png(_app *p_app) {
	bytes data = p_app->file.data;
	read_u64_be(&data);
	_png_info *p_info = calloc(1, sizeof(_png_info));
	read_png_ihdr(p_app, &data, p_info);

	while (1) {
		u32 length = read_u32_be(&data);
		u32 type = read_u32_be(&data);

		switch (type) {
			case PNG_CHUNK_IDAT: {
				p_info->idat_buf = realloc(p_info->idat_buf, p_info->idat_size + length);
				memcpy(p_info->idat_buf + p_info->idat_size, data, length);
				p_info->idat_size += length;
				data += length;
				break;
			}
			case PNG_CHUNK_PLTE: {
				p_info->palette_size = length / 3;
				for (u32 i = 0; i < p_info->palette_size; i++) {
					p_info->palette[i * 3 + 0] = read_u8(&data);
					p_info->palette[i * 3 + 1] = read_u8(&data);
					p_info->palette[i * 3 + 2] = read_u8(&data);
				}
				break;
			}
			case PNG_CHUNK_tRNS: {
				p_info->has_tRNS = true;
				switch (p_info->colour_type) {
					case 0: {
						p_info->tRNS_gray = read_u16_be(&data);
						break;
					}
					case 2: {
						p_info->tRNS_rgb[0] = read_u16_be(&data);
						p_info->tRNS_rgb[1] = read_u16_be(&data);
						p_info->tRNS_rgb[2] = read_u16_be(&data);
						break;
					}
					case 3: {
						for (u32 i = 0; i < length; i++)
							p_info->tRNS_alpha[i] = read_u8(&data);
						break;
					}
				}
				break;
			}
			case PNG_CHUNK_gAMA: {
				p_info->has_gAMA = true;
				p_info->gamma = read_u32_be(&data) / 100000.0f;
				break;
			}
			case PNG_CHUNK_IEND: {
				p_app->file.info = p_info;
				return;
			}
			default: {
				data += length;
				break;
			}
		}
		read_u32_be(&data);
	}
}

static void decode_png(_app *p_app) {
	_png_info *p_info = (_png_info *)p_app->file.info;
	p_info->out_size = p_app->output.height * (1 + p_app->output.width * p_info->channels);
	p_info->out = malloc(p_info->out_size);

	z_stream zs = {0};
	inflateInit(&zs);

	zs.next_in   = p_info->idat_buf;
	zs.avail_in  = p_info->idat_size;
	zs.next_out  = p_info->out;
	zs.avail_out = p_info->out_size;

	int ret = inflate(&zs, Z_FINISH);
	inflateEnd(&zs);

	if (ret != Z_STREAM_END) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"png decode => zlib decompression failed"
		);
		exit(1);
	}

	free(p_info->idat_buf);
	p_info->idat_buf = NULL;
}

static inline u8 paeth(u8 a, u8 b, u8 c) {
	i32 p  = (i32)a + (i32)b - (i32)c;
	i32 pa = abs(p - (i32)a);
	i32 pb = abs(p - (i32)b);
	i32 pc = abs(p - (i32)c);
	if (pa <= pb && pa <= pc) return a;
	if (pb <= pc)             return b;
	return c;
}

static void unfilter_png(_app *p_app) {
	_png_info *p_info = (_png_info *)p_app->file.info;
	usize stride = p_info->stride;
	usize bpp = p_info->channels;

	u8 *out = malloc(p_app->output.height * stride);
	u8 *prev_buf = calloc(stride, 1);

	p_app->output.pixels = out;

	u8 *filtered = p_info->out;

	for (u32 y = 0; y < p_app->output.height; y++) {
		u8 filter = *filtered++;
		u8 *dst = out + y * stride;

		for (usize x = 0; x < stride; x++) {
			u8 raw = *filtered++;

			switch (filter) {
				case 0:
					dst[x] = raw;
					break;

				case 1: {
					u8 a = x >= bpp ? dst[x - bpp] : 0;
					dst[x] = raw + a;
					break;
					}	
				case 2: {
					u8 b = prev_buf[x];
					dst[x] = raw + b;
					break;
					}
				case 3: {
					u8 a = x >= bpp ? dst[x - bpp] : 0;
					u8 b = prev_buf[x];
					dst[x] = raw + (a + b) / 2;
					break;
					}
				case 4: {
					u8 a = x >= bpp ? dst[x - bpp] : 0;
					u8 b = prev_buf[x];
					u8 c = x >= bpp ? prev_buf[x - bpp] : 0;
					dst[x] = raw + paeth(a, b, c);
					break;
					}
			}
		}

		memcpy(prev_buf, dst, stride);
	}

	free(prev_buf);
	free(p_info->out);
	p_info->out = NULL;

	if (p_info->colour_type == 3) {
		usize n = (usize)p_app->output.width * p_app->output.height;
		u8 *rgb = malloc(n * 3);

		for (usize i = 0; i < n; i++) {
			u8 idx = p_app->output.pixels[i];
			rgb[i*3 + 0] = p_info->palette[idx*3 + 0];
			rgb[i*3 + 1] = p_info->palette[idx*3 + 1];
			rgb[i*3 + 2] = p_info->palette[idx*3 + 2];
		}

		free(p_app->output.pixels);
		p_app->output.pixels = rgb;
		p_info->channels = 3;
		p_info->stride = p_app->output.width * 3;
	}
}

static void read_jpg(_app *p_app) {
	printf("jpg\n");
}
static void read_webp(_app *p_app) {
	printf("webp\n");
}

void file_decode(_app *p_app) {
	switch (p_app->file.type) {
		case _FILE_TYPE_PNG:  
			read_png(p_app);
			decode_png(p_app);
			unfilter_png(p_app);
			break;
		case _FILE_TYPE_JPG: 
			read_jpg(p_app); 
			break;
		case _FILE_TYPE_WEBP: 
			read_webp(p_app); 
			break;
	}
}
