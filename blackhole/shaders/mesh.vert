#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_norm;
layout(location = 3) in uvec4 in_data;

layout(location = 0) out vec3 frag_pos;
layout(location = 1) out vec2 frag_uv;
layout(location = 2) out vec3 frag_norm;
layout(location = 3) out uvec4 frag_data;
layout(location = 4) flat out uint frag_object_index;

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

layout(std430, binding = 2) readonly buffer _sbo_solar_objects {
    uint solar_object_count;
    uint _pad[3];
    _solar_object solar_objects[];
} sbo_solar_objects;

layout(push_constant) uniform push_constants {
    uint object_index;
} pc;

void main() {
    _solar_object obj = sbo_solar_objects.solar_objects[pc.object_index];

    vec3 world_pos = in_pos * obj.radius + obj.position;

    gl_Position = ubo.proj * ubo.view * vec4(world_pos, 1.0);

    frag_pos = world_pos;
    frag_norm = normalize(in_norm);
    frag_uv = in_uv;
    frag_data = uvec4(obj.colour_id, in_data.y, in_data.z, in_data.w);
    frag_object_index = pc.object_index;
}
