#include "headers/define.h"
#include "headers/validation.h"
#include "headers/helper.h"
#include "headers/png.h"

static void read_png_ihdr(_app *p_app, bytes *p_data, _png_info *p_info) {
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
						memset(p_info->tRNS_alpha, 255, sizeof(p_info->tRNS_alpha));
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

	usize n = (usize)p_app->output.width * p_app->output.height;
	u8 *rgba = malloc(n * 4);
	u8 *src = p_app->output.pixels;

	switch (p_info->colour_type) {
		case 0: {
			u8 trns = (u8)p_info->tRNS_gray;
			for (usize i = 0; i < n; i++) {
				u8 g = src[i];
				rgba[i*4 + 0] = g;
				rgba[i*4 + 1] = g;
				rgba[i*4 + 2] = g;
				rgba[i*4 + 3] = (p_info->has_tRNS && g == trns) ? 0 : 255;
			}
			break;
		}
		case 2: {
			u8 tr = (u8)p_info->tRNS_rgb[0];
			u8 tg = (u8)p_info->tRNS_rgb[1];
			u8 tb = (u8)p_info->tRNS_rgb[2];
			for (usize i = 0; i < n; i++) {
				u8 r = src[i*3 + 0];
				u8 g = src[i*3 + 1];
				u8 b = src[i*3 + 2];
				rgba[i*4 + 0] = r;
				rgba[i*4 + 1] = g;
				rgba[i*4 + 2] = b;
				rgba[i*4 + 3] = (p_info->has_tRNS && r == tr && g == tg && b == tb) ? 0 : 255;
			}
			break;
		}
		case 3: {
			for (usize i = 0; i < n; i++) {
				u8 idx = src[i];
				rgba[i*4 + 0] = p_info->palette[idx*3 + 0];
				rgba[i*4 + 1] = p_info->palette[idx*3 + 1];
				rgba[i*4 + 2] = p_info->palette[idx*3 + 2];
				rgba[i*4 + 3] = p_info->has_tRNS ? p_info->tRNS_alpha[idx] : 255;
			}
			break;
		}
		case 4: {
			for (usize i = 0; i < n; i++) {
				u8 g = src[i*2 + 0];
				rgba[i*4 + 0] = g;
				rgba[i*4 + 1] = g;
				rgba[i*4 + 2] = g;
				rgba[i*4 + 3] = src[i*2 + 1];
			}
			break;
		}
		case 6: {
			memcpy(rgba, src, n * 4);
			break;
		}
	}

	free(p_app->output.pixels);
	p_app->output.pixels = rgba;
	p_info->channels = 4;
	p_info->stride = p_app->output.width * 4;
}

void png(_app *p_app) {
	read_png(p_app);
	decode_png(p_app);
	unfilter_png(p_app);
}
