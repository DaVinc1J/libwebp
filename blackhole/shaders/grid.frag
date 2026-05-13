#version 450

layout(location = 0) in float frag_intensity;
layout(location = 1) in vec3 frag_world_pos;

layout(location = 0) out vec4 frag_colour;

layout(set = 0, binding = 0) uniform _ubo {
    mat4 proj;
    mat4 view;
    mat4 inv_proj;
    mat4 inv_view;
    vec4 ambient;
    vec4 grid_params;
} ubo;

void main() {
    const float FADE_START = 200.0;
    const float FADE_END = 500.0;

    vec3 cool = vec3(0.2, 0.3, 0.8);
    vec3 hot = vec3(1.0, 0.2, 0.0);
    vec3 colour = mix(cool, hot, frag_intensity);

    vec3 cam = ubo.inv_view[3].xyz;
    float frag_dist = length((frag_world_pos - cam).xz);
    float alpha = 1.0 - smoothstep(FADE_START, FADE_END, frag_dist);

    if (alpha < 0.01) discard;
    frag_colour = vec4(colour, alpha);
}
