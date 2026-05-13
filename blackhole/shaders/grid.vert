#version 450

layout(location = 0) in vec3 in_pos;

layout(location = 0) out float frag_intensity;
layout(location = 1) out vec3 frag_world_pos;

layout(set = 0, binding = 0) uniform _ubo {
    mat4 proj;
    mat4 view;
    mat4 inv_proj;
    mat4 inv_view;
    vec4 ambient;
    vec4 grid_params;
} ubo;

struct _solar_object {
    vec3 position;
    float _pad0;
    vec3 velocity;
    float _pad1;
    vec3 acceleration;
    float _pad2;
    float mass;
    float radius;
    uint colour_id;
    uint billboard_index;
    uint type;
    uint planet_type;
    float intensity;
    float schwarzschild_radius;
};

layout(std430, set = 0, binding = 2) readonly buffer _sbo_solar_objects {
    uint solar_object_count;
    uint _pad[3];
    _solar_object solar_objects[];
} sbo_solar_objects;

float compute_displacement(vec2 xz, float gravity_scale, float softening) {
    float raw = 0.0;
    uint count = sbo_solar_objects.solar_object_count;
    for (uint i = 0u; i < count; ++i) {
        _solar_object obj = sbo_solar_objects.solar_objects[i];
        float r = length(xz - obj.position.xz);
        raw -= gravity_scale * obj.mass / (r + softening);
    }

    float compressed = (log(1 - raw));

    return -(compressed * compressed);
}

void main() {
    float gravity_scale = ubo.grid_params.x;
    float softening = ubo.grid_params.y;
    float max_depth = ubo.grid_params.z;
    float max_radius = ubo.grid_params.w;

    vec2 xz = in_pos.xz;
    float disp = compute_displacement(xz, gravity_scale, softening);
    float disp_height = disp - max_radius;
    vec3 world = vec3(xz.x, disp_height, xz.y);

    frag_intensity = clamp(-disp / max_depth, 0.0, 1.0);
    frag_world_pos = world;
    gl_Position = ubo.proj * ubo.view * vec4(world, 1.0);
}
