#version 450

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_colour;

layout(set = 0, binding = 0) uniform sampler2D u_texture;

layout(push_constant) uniform PC {
    vec2 uv_scale;
    vec2 border_uv;
    vec4 border_colour;
    vec4 background_colour;
} pc;

void main() {
    vec2 uv = (in_uv - 0.5) / pc.uv_scale + 0.5;

    vec4 base;

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        base = pc.background_colour;
    }

    if (in_uv.x < pc.border_uv.x || in_uv.x > 1.0 - pc.border_uv.x ||
            in_uv.y < pc.border_uv.y || in_uv.y > 1.0 - pc.border_uv.y) {
        if (pc.border_colour.a > 0.0) {
            out_colour = vec4(
                    mix(base.rgb, pc.border_colour.rgb, pc.border_colour.a),
                    max(base.a, pc.border_colour.a)
                );
            return;
        }
    }
    out_colour = base;
}
