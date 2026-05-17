#include "headers/jpg.h"
#include "headers/define.h"
#include "headers/validation.h"
#include "headers/helper.h"

static void read_jpg(_app *p_app) {
	_jpg_info *p_info = calloc(1, sizeof(_jpg_info));

	bytes data = p_app->file.data;
	skip_bytes(&data, 2); // SOI MARKER

	if (read_u16_be(&data) != JPG_APP_0) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"jpg read => invalid jpg order -> NO APP-0 AFTER SOI"
		);
		exit(1);
	}

	u16 length = copy_u16_be(data);
	skip_bytes(&data, length); // SKIP APP-0

	while (1) {
		u16 marker = read_u16_be(&data);

		if (marker == JPG_SOI) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"jpg read => invalid jpg order -> replicated SOI"
			);
			exit(1);
		}

		u16 length = copy_u16_be(data);

		switch (marker) {
			case JPG_DQT: {
				skip_bytes(&data, 2);

				i32 len = (i32)length;
				while (len > 2) {
					u8 pr_id = read_u8(&data);
					u8 precision = pr_id >> 4;
					u8 id = pr_id & 0x0f;
					len--;

					if (precision == 1) {
						submit_debug_message(
							p_app->inst.instance,
							VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
							"jpg read => 16-bit quantization tables not supported"
						);
						exit(1);
					}

					for (i32 i = 0; i < 64; i++) {
						p_info->qt[id][i] = read_u8(&data);
					}
					p_info->qt_set[id] = true;
					len -= 64;
				}
				break;
			}

			case JPG_SOF_0: {
				skip_bytes(&data, 2);
				u8 precision = read_u8(&data);

				if (precision != 8) {
					submit_debug_message(
						p_app->inst.instance,
						VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
						"jpg read => only 8-bit precision is supported"
					);
					exit(1);
				}

				p_app->output.height = (u32)read_u16_be(&data);
				p_app->output.width = (u32)read_u16_be(&data);

				p_info->comp_count = read_u8(&data);

				for (u8 i = 0; i < p_info->comp_count; i++) {
					p_info->comp[i].id = read_u8(&data);
					u8 sampling_factor = read_u8(&data);
					p_info->comp[i].h_samp = sampling_factor >> 4;
					p_info->comp[i].v_samp = sampling_factor & 0x0f;
					p_info->comp[i].qt_id = read_u8(&data);
				}

				break;
			}

			case JPG_DHT: {
				skip_bytes(&data, 2);

				u8 tp_id = read_u8(&data);

				u8 type = tp_id >> 4;
				u8 id = tp_id & 0x0f;

				i32 total_symbols = 0;
				for (i32 i = 0; i < 16; i++) {
					p_info->ht[type][id].count[i] = read_u8(&data);
					total_symbols += p_info->ht[type][id].count[i];
				}

				p_info->ht[type][id].symbol_count = total_symbols;
				for (i32 i = 0; i < total_symbols; i++) {
					p_info->ht[type][id].symbols[i] = read_u8(&data);
				}

				p_info->ht_set[type][id] = true;
				break;
			}

			case JPG_SOS: {
				skip_bytes(&data, 2);

				u8 comp_count = read_u8(&data);
				if (p_info->comp_count != comp_count) {
					submit_debug_message(
						p_app->inst.instance,
						VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
						"jpg read => SOS component count does not match SOF-0, multi-scan not supported"
					);
					exit(1);
				}

				for (u8 i = 0; i < p_info->comp_count; i++) {
					u8 id = read_u8(&data);
					u8 ht = read_u8(&data);

					for (u8 j = 0; j < p_info->comp_count; j++) {
						if (p_info->comp[j].id == id) {
							p_info->comp[j].ht_id_dc = ht >> 4;
							p_info->comp[j].ht_id_ac = ht & 0x0f;
							break;
						}
					}
				}

				skip_bytes(&data, 3);

				p_info->scan_data = data;
    		p_info->scan_size = (p_app->file.data + p_app->file.data_size) - data - 2;

				p_app->file.info = p_info;
				return;
			}

			default: {
				skip_bytes(&data, length);
				break;
			}
		}
	}
}

void jpg(_app *p_app) {
	read_jpg(p_app);
}
