#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 frag_pos;
layout(location = 1) in vec2 frag_uv;
layout(location = 2) in vec3 frag_norm;
layout(location = 3) in flat uvec4 frag_data;
layout(location = 4) in flat uint frag_object_index;

layout(location = 0) out vec4 out_colour;

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
    _solar_object objects[];
} sbo_solar_objects;

const uint SOLAR_OBJECT_TYPE_PLAIN = 0u;
const uint SOLAR_OBJECT_TYPE_LIGHT_EMIT = 1u;

vec3 unpack_colour(uint packed_colour) {
    return vec3(
        float((packed_colour >> 16) & 0xFF) / 255.0,
        float((packed_colour >> 8) & 0xFF) / 255.0,
        float(packed_colour & 0xFF) / 255.0
    );
}

void main() {
    vec3 surface_colour = unpack_colour(frag_data[0]);
    _solar_object self = sbo_solar_objects.objects[frag_object_index];

    if (self.type == SOLAR_OBJECT_TYPE_LIGHT_EMIT) {
        out_colour = vec4(surface_colour, 1.0);
        return;
    }

    vec3 diffuse_light = ubo.ambient.xyz * ubo.ambient.w;
    vec3 surface_normal = normalize(frag_norm);

    for (uint i = 0u; i < sbo_solar_objects.solar_object_count; i++) {
        if (i == frag_object_index) continue;

        _solar_object light = sbo_solar_objects.objects[i];
        if (light.type != SOLAR_OBJECT_TYPE_LIGHT_EMIT) continue;

        vec3 light_direction = light.position - frag_pos;

        float distance_sq = dot(light_direction, light_direction);
        float attenuation = 1.0 / max(distance_sq, 0.0001);

        light_direction = normalize(light_direction);

        float cos_ang_incidence = max(dot(surface_normal, light_direction), 0.0);

        vec3 light_colour = unpack_colour(light.colour_id);
        vec3 intensity = light_colour * light.intensity * attenuation;

        diffuse_light += intensity * cos_ang_incidence;
    }

    vec3 diffuse_colour = diffuse_light * surface_colour;

    out_colour = vec4(diffuse_colour, 1.0);
}
