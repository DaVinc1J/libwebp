#version 450

layout(location = 0) in vec2 frag_offset;
layout(location = 1) in vec4 frag_size_rotation;
layout(location = 2) in flat uvec4 frag_type_data;
layout(location = 3) in flat uvec4 frag_flags;
layout(location = 4) in float frag_alpha;

layout(location = 0) out vec4 out_colour;

void main() {
    uint type_flags = frag_flags.x;
    uint shape_flags = frag_flags.y;
    uint location_flags = frag_flags.z;

    bool is_light = (type_flags & 1u) != 0u;
    bool is_square = (shape_flags & 1u) != 0u;

    uint packed_color = frag_type_data.x;
    vec3 color = vec3(
            float((packed_color >> 16) & 0xFF) / 255.0,
            float((packed_color >> 8) & 0xFF) / 255.0,
            float(packed_color & 0xFF) / 255.0
        );

    float border_alpha = 1.0;

    if (is_square) {
        float dist = max(abs(frag_offset.x), abs(frag_offset.y));

        if (dist > 1.0) discard;

        if (is_light) {
            float inner = 0.7;
            float outer = 1.0;

            if (dist > inner) {
                float t = (dist - inner) / (outer - inner);
                border_alpha = 1.0 - t * t;
            }
        }
    } else {
        float dist = length(frag_offset);

        if (dist > 1.0) discard;

        if (is_light) {
            float inner = 0.7;
            float outer = 1.0;

            if (dist > inner) {
                float t = (dist - inner) / (outer - inner);
                border_alpha = 1.0 - t * t;
            }
        }
    }

    if (is_light) {
        out_colour = vec4(color, border_alpha);
    } else {
        out_colour = vec4(color, frag_alpha);
    }
}
