#include "headers/jpg.h"
#include "headers/define.h"
#include "headers/validation.h"
#include "headers/helper.h"

#define HT_NODE_NIL (-1)
#define HT_INITIAL_NODE_CAP 64

static const u8 zigzag[64] = {
	 0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63,
};

static u8 read_bit(_bit_reader *br) {
	if (br->byte_off >= br->size) return 0;
	u8 byte = br->data[br->byte_off];
	u8 bit  = (byte >> (7 - br->bit_off)) & 1;

	br->bit_off++;
	if (br->bit_off == 8) {
		br->bit_off = 0;
		br->byte_off++;
		if (br->byte_off < br->size
		    && br->data[br->byte_off - 1] == 0xFF
		    && br->data[br->byte_off] == 0x00) {
			br->byte_off++;
		}
	}
	return bit;
}

static u32 read_bits(_bit_reader *br, u8 n) {
	u32 v = 0;
	for (u8 i = 0; i < n; i++) {
		v = (v << 1) | read_bit(br);
	}
	return v;
}

static u8 decode_symbol(_jpg_info *p_info, u8 type, u8 id, _bit_reader *br) {
	i32 idx = p_info->ht_root.raw[type][id];
	_huffman_tree_node *nodes = p_info->ht_nodes.raw[type][id];
	while (!nodes[idx].leaf) {
		u8 bit = read_bit(br);
		idx = nodes[idx].child[bit];
		if (idx == HT_NODE_NIL) return 0;
	}
	return (u8)nodes[idx].value;
}

// JPEG magnitude sign-extension: a `cat`-bit magnitude with MSB=1 is positive,
// with MSB=0 is negative (value - (2^cat - 1)).
static i32 jpg_extend(u32 bits, u8 cat) {
	if (cat == 0) return 0;
	i32 v_t = 1 << (cat - 1);
	if ((i32)bits < v_t) {
		return (i32)bits - ((1 << cat) - 1);
	}
	return (i32)bits;
}

static void decode_block(_jpg_info *p_info, _bit_reader *br, u8 dc_ht_id, u8 ac_ht_id, i32 *dc_pred, i32 out[64]) {
	memset(out, 0, 64 * sizeof(i32));

	// DC: one symbol = magnitude category; then `cat` magnitude bits.
	u8  dc_cat  = decode_symbol(p_info, 0 /*DC*/, dc_ht_id, br);
	u32 dc_bits = read_bits(br, dc_cat);
	*dc_pred += jpg_extend(dc_bits, dc_cat);
	out[zigzag[0]] = *dc_pred;

	// AC (positions 1..63 in zig-zag order).
	i32 k = 1;
	while (k < 64) {
		u8 sym = decode_symbol(p_info, 1 /*AC*/, ac_ht_id, br);
		if (sym == 0x00) break;                 // EOB: rest of block is zero
		if (sym == 0xF0) { k += 16; continue; } // ZRL: 16 zeros

		u8 run = sym >> 4;
		u8 cat = sym & 0x0F;
		k += run;
		if (k >= 64) break;

		u32 ac_bits = read_bits(br, cat);
		out[zigzag[k]] = jpg_extend(ac_bits, cat);
		k++;
	}
}

static void decode_scan(_jpg_info *p_info, _app *p_app) {
	// mcu_data is i32[mcu][component][64] -- one 8x8 block per component per MCU.
	// That layout only fits 4:4:4 (no chroma subsampling). Bail otherwise.
	for (u8 i = 0; i < p_info->comp_count; i++) {
		if (p_info->comp[i].h_samp != 1 || p_info->comp[i].v_samp != 1) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"jpg decode => chroma subsampling not supported (h_samp/v_samp must both be 1)"
			);
			exit(1);
		}
	}

	u32 mcus_x = (p_app->output.width  + 7) / 8;
	u32 mcus_y = (p_app->output.height + 7) / 8;
	p_info->mcu_count = (i32)(mcus_x * mcus_y);
	p_info->mcu_data  = calloc((usize)p_info->mcu_count, sizeof(*p_info->mcu_data));

	_bit_reader br = { p_info->scan_data, p_info->scan_size, 0, 0 };

	p_info->dc_pred[0] = 0;
	p_info->dc_pred[1] = 0;
	p_info->dc_pred[2] = 0;

	for (i32 m = 0; m < p_info->mcu_count; m++) {
		for (u8 c = 0; c < p_info->comp_count; c++) {
			decode_block(
				p_info, &br,
				p_info->comp[c].ht_id.dc,
				p_info->comp[c].ht_id.ac,
				&p_info->dc_pred[c],
				p_info->mcu_data[m][c]
			);
		}
	}
}

static void init_huffman_trees(_jpg_info *p_info) {
	for (u8 type = 0; type < 2; type++) {
		for (u8 id = 0; id < 2; id++) {
			p_info->ht_set.raw[type][id] = false;
			p_info->ht_root.raw[type][id] = HT_NODE_NIL;
			p_info->ht_node_count.raw[type][id] = 0;
			p_info->ht_node_cap.raw[type][id] = HT_INITIAL_NODE_CAP;
			p_info->ht_nodes.raw[type][id] = calloc(
				HT_INITIAL_NODE_CAP, sizeof(_huffman_tree_node)
			);
		}
	}
}

void cleanup_jpg(_app *p_app) {
	_jpg_info *p_info = (_jpg_info *)p_app->file.info;

	for (u8 type = 0; type < 2; type++) {
		for (u8 id = 0; id < 2; id++) {
			free(p_info->ht_nodes.raw[type][id]);
		}
	}

	free(p_info);
}

static void ensure_node_cap(_jpg_info *p_info, u8 type, u8 id, i32 need) {
	i32 cap = p_info->ht_node_cap.raw[type][id];
	if (need <= cap) {
		return;
	}

	i32 new_cap = cap;
	while (new_cap < need) {
		new_cap *= 2;
	}

	p_info->ht_nodes.raw[type][id] = realloc(p_info->ht_nodes.raw[type][id],(usize)new_cap * sizeof(_huffman_tree_node));
	p_info->ht_node_cap.raw[type][id] = new_cap;
}

static i32 create_node(_jpg_info *p_info, u8 type, u8 id, bool8 is_root) {
	i32 idx = p_info->ht_node_count.raw[type][id];
	ensure_node_cap(p_info, type, id, idx + 1);
	p_info->ht_node_count.raw[type][id] = idx + 1;

	_huffman_tree_node *n = &p_info->ht_nodes.raw[type][id][idx];
	n->root = is_root;
	n->leaf = false;
	n->code[0] = '\0';
	n->value = 0;
	n->child[0] = HT_NODE_NIL;
	n->child[1] = HT_NODE_NIL;
	n->parent = HT_NODE_NIL;
	return idx;
}

static void insert(_jpg_info *p_info, u8 type, u8 id, i32 parent, _dir direction) {
	if (parent == HT_NODE_NIL)
		return;

	if (p_info->ht_nodes.raw[type][id][parent].child[direction] != HT_NODE_NIL)
		return;

	char parent_code[17];
	memcpy(parent_code, p_info->ht_nodes.raw[type][id][parent].code, 17);

	i32 child = create_node(p_info, type, id, false);
	_huffman_tree_node *nodes = p_info->ht_nodes.raw[type][id];

	nodes[parent].child[direction] = child;

	nodes[child].parent = parent;

	i32 len = (i32)strlen(parent_code);
	if (len < 16) {
		memcpy(nodes[child].code, parent_code, (usize)len);
		nodes[child].code[len] = '0' + direction;
		nodes[child].code[len + 1] = '\0';
	}
}

static void build_huffman_tree(_jpg_info *p_info, u8 type, u8 id) {
	_huffman_tree *tree = &p_info->ht.raw[type][id];

	i32 root = create_node(p_info, type, id, true);

	p_info->ht_root.raw[type][id] = root;
	insert(p_info, type, id, root, LEFT);
	insert(p_info, type, id, root, RIGHT);

	i32 level_start = root + 1;
	i32 level_end = p_info->ht_node_count.raw[type][id];
	i32 sym_off = 0;

	for (i32 i = 1; i <= 16; i++) {
		i32 count = tree->count[i - 1];
		i32 new_start = p_info->ht_node_count.raw[type][id];

		for (i32 s = level_start; s < level_end; s++) {
			if (s - level_start < count) {
				_huffman_tree_node *nodes = p_info->ht_nodes.raw[type][id];
				nodes[s].value = tree->symbols[sym_off++];
				nodes[s].leaf  = true;
				continue;
			}

			insert(p_info, type, id, s, LEFT);
			insert(p_info, type, id, s, RIGHT);
		}
		level_start = new_start;
		level_end = p_info->ht_node_count.raw[type][id];
	}
}

static void read_jpg(_app *p_app) {
	_jpg_info *p_info = calloc(1, sizeof(_jpg_info));
	init_huffman_trees(p_info);

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
							"jpg read => 16-bit quantization trees not supported"
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

			case JPG_SOF_1:
				submit_debug_message(
					p_app->inst.instance,
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					"jpg read => SOF1 extended sequential DCT not supported"
				);
				exit(1);

			case JPG_SOF_2:
				submit_debug_message(
					p_app->inst.instance,
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					"jpg read => SOF2 progressive DCT not supported"
				);
				exit(1);

			case JPG_SOF_3:
				submit_debug_message(
					p_app->inst.instance,
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					"jpg read => SOF3 lossless sequential not supported"
				);
				exit(1);

			case JPG_SOF_5:
				submit_debug_message(
					p_app->inst.instance,
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					"jpg read => SOF5 differential sequential DCT not supported"
				);
				exit(1);

			case JPG_SOF_6:
				submit_debug_message(
					p_app->inst.instance,
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					"jpg read => SOF6 differential progressive DCT not supported"
				);
				exit(1);

			case JPG_SOF_7:
				submit_debug_message(
					p_app->inst.instance,
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					"jpg read => SOF7 differential lossless sequential not supported"
				);
				exit(1);

			case JPG_SOF_9:
				submit_debug_message(
					p_app->inst.instance,
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					"jpg read => SOF9 arithmetic coded extended sequential DCT not supported"
				);
				exit(1);

			case JPG_SOF_10:
				submit_debug_message(
					p_app->inst.instance,
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					"jpg read => SOF10 arithmetic coded progressive DCT not supported"
				);
				exit(1);

			case JPG_SOF_11:
				submit_debug_message(
					p_app->inst.instance,
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					"jpg read => SOF11 arithmetic coded lossless sequential not supported"
				);
				exit(1);

			case JPG_SOF_13:
				submit_debug_message(
					p_app->inst.instance,
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					"jpg read => SOF13 arithmetic coded differential sequential DCT not supported"
				);
				exit(1);

			case JPG_SOF_14:
				submit_debug_message(
					p_app->inst.instance,
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					"jpg read => SOF14 arithmetic coded differential progressive DCT not supported"
				);
				exit(1);

			case JPG_SOF_15:
				submit_debug_message(
					p_app->inst.instance,
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					"jpg read => SOF15 arithmetic coded differential lossless sequential not supported"
				);
				exit(1);

			case JPG_DHT: {
				skip_bytes(&data, 2);

				u8 tp_id = read_u8(&data);

				u8 type = tp_id >> 4;
				u8 id = tp_id & 0x0f;

				i32 total_symbols = 0;
				for (i32 i = 0; i < 16; i++) {
					p_info->ht.raw[type][id].count[i] = read_u8(&data);
					total_symbols += p_info->ht.raw[type][id].count[i];
				}

				p_info->ht.raw[type][id].symbol_count = total_symbols;
				for (i32 i = 0; i < total_symbols; i++) {
					p_info->ht.raw[type][id].symbols[i] = read_u8(&data);
				}

				p_info->ht_set.raw[type][id] = true;
				build_huffman_tree(p_info, type, id);
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
							p_info->comp[j].ht_id.dc = ht >> 4;
							p_info->comp[j].ht_id.ac = ht & 0x0f;
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
