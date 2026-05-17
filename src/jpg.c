#include "headers/jpg.h"
#include "headers/define.h"
#include "headers/validation.h"
#include "headers/helper.h"

#define HT_NODE_NIL (-1)

static void ensure_node_cap(_jpg_info *p_info, u8 type, u8 id, i32 need) {

	i32 cap = p_info->ht_node_cap.raw[type][id];
	if (need <= cap) {
		return;
	}

	i32 new_cap = cap;
	while (new_cap < need) {
		new_cap *= 2;
	}

	p_info->ht_nodes.raw[type][id] = realloc(p_info->ht_nodes.raw[type][id],(usize)new_cap * sizeof(_huffman_table_node));
	p_info->ht_node_cap.raw[type][id] = new_cap;
	p_info->ht_node_count.raw[type][id] = need;
}

static i32 create_node(_jpg_info *p_info, u8 type, u8 id, bool8 is_root) {
	i32 idx = p_info->ht_node_count.raw[type][id];
	ensure_node_cap(p_info, type, id, idx + 1);

	_huffman_table_node *n = &p_info->ht_nodes.raw[type][id][idx];
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
	if (parent == HT_NODE_NIL) return;

	if (p_info->ht_nodes.raw[type][id][parent].child[direction] != HT_NODE_NIL) return;

	char parent_code[17];
	memcpy(parent_code, p_info->ht_nodes.raw[type][id][parent].code, 17);

	i32 child = create_node(p_info, type, id, false);
	_huffman_table_node *nodes = p_info->ht_nodes.raw[type][id];

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
	_huffman_table *table = &p_info->ht.raw[type][id];
	printf("hello1\n");

	i32 root = create_node(p_info, type, id, true);

	printf("hello2\n");
	p_info->ht_root.raw[type][id] = root;
	insert(p_info, type, id, root, LEFT);
	insert(p_info, type, id, root, RIGHT);

	i32 level_start = root + 1;
	i32 level_end = p_info->ht_node_count.raw[type][id];
	printf("%d\n", level_end);
	i32 sym_off = 0;

	for (i32 i = 1; i <= 16; i++) {
		i32 count = table->count[i - 1];
		i32 new_start = p_info->ht_node_count.raw[type][id];

		for (i32 s = level_start; s < level_end; s++) {
			if (s - level_start < count) {
				_huffman_table_node *nodes = p_info->ht_nodes.raw[type][id];
				nodes[s].value = table->symbols[sym_off++];
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
