#version 450

layout(location = 0) in vec2 v_uv;
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

layout(std430, binding = 1) readonly buffer _sbo_solar_objects {
    uint solar_object_count;
    uint _pad[3];
    _solar_object objects[];
} sbo_solar_objects;

layout(set = 0, binding = 2) uniform sampler2D scene_tex;

const uint SOLAR_OBJECT_TYPE_BLACKHOLE = 3u;
const float INFLUENCE_FACTOR = 60.0;
const int MAX_RK4_STEPS = 384;
const float ESCAPE_U_FACTOR = 0.003;
const float DPHI_BASE = 0.022;
const float DPHI_JITTER = 0.05;

const float DISK_INNER_RS = 3.0;
const float DISK_OUTER_RS = 11.0;

uint seed;

float random() {
    seed = seed * 1103515245u + 12345u;
    return float(seed & 0x7fffffffu) / float(0x7fffffffu);
}

float random_range(float min_val, float max_val) {
    return min_val + random() * (max_val - min_val);
}

vec2 geodesic_rhs(vec2 s, float rs) {
    return vec2(s.y, 1.5 * rs * s.x * s.x - s.x);
}

vec2 rk4_step(vec2 s, float h, float rs) {
    vec2 k1 = geodesic_rhs(s, rs);
    vec2 k2 = geodesic_rhs(s + 0.5 * h * k1, rs);
    vec2 k3 = geodesic_rhs(s + 0.5 * h * k2, rs);
    vec2 k4 = geodesic_rhs(s + h * k3, rs);
    return s + (h / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
}

vec3 disk_colour(float r_rs, float phi_world) {
    float t = clamp((r_rs - DISK_INNER_RS) / (DISK_OUTER_RS - DISK_INNER_RS), 0.0, 1.0);
    vec3 hot = vec3(1.20, 1.05, 0.85);
    vec3 mid = vec3(1.10, 0.55, 0.18);
    vec3 cool = vec3(0.55, 0.18, 0.05);
    vec3 c = mix(hot, mid, smoothstep(0.0, 0.45, t));
    c = mix(c, cool, smoothstep(0.45, 1.0, t));
    float swirl = 0.5 + 0.5 * sin(phi_world * 6.0 + r_rs * 1.7);
    float edge = exp(-t * 4.0);
    return c * mix(0.7, 1.15, swirl) * (0.9 + 0.6 * edge);
}

void main() {
    seed = uint(v_uv.x * 1234567.0 + v_uv.y * 7890123.0) ^ 123456789u;

    vec2 ndc = v_uv * 2.0 - 1.0;
    vec4 view_h = ubo.inv_proj * vec4(ndc, 1.0, 1.0);
    vec3 view_dir = view_h.xyz / view_h.w;
    vec3 world_dir = normalize((ubo.inv_view * vec4(view_dir, 0.0)).xyz);
    vec3 cam_pos = (ubo.inv_view * vec4(0.0, 0.0, 0.0, 1.0)).xyz;

    int best_bh = -1;
    float best_score = 0.0;
    for (uint i = 0u; i < sbo_solar_objects.solar_object_count; ++i) {
        _solar_object obj = sbo_solar_objects.objects[i];
        if (obj.type != SOLAR_OBJECT_TYPE_BLACKHOLE) continue;
        vec3 to_bh = obj.position - cam_pos;
        float r0 = length(to_bh);
        if (r0 < obj.schwarzschild_radius) {
            out_colour = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
        float fwd = dot(to_bh, world_dir);
        if (fwd <= 0.0) continue;
        float b = length(to_bh - fwd * world_dir);
        if (b > obj.schwarzschild_radius * INFLUENCE_FACTOR) continue;
        float score = obj.schwarzschild_radius / max(b * b, 1e-6);
        if (score > best_score) {
            best_score = score;
            best_bh = int(i);
        }
    }

    if (best_bh < 0) {
        out_colour = texture(scene_tex, v_uv);
        return;
    }

    _solar_object bh = sbo_solar_objects.objects[best_bh];
    float rs = bh.schwarzschild_radius;
    vec3 to_bh = bh.position - cam_pos;
    float r0 = length(to_bh);
    vec3 e_r = -to_bh / r0;
    vec3 pn = cross(e_r, world_dir);
    float nlen = length(pn);
    if (nlen < 1e-6) {
        out_colour = (dot(to_bh, world_dir) > 0.0) ? vec4(0.0, 0.0, 0.0, 1.0) : texture(scene_tex, v_uv);
        return;
    }
    pn /= nlen;
    vec3 e_phi = cross(pn, e_r);
    float v_r = dot(world_dir, e_r);
    float v_phi = dot(world_dir, e_phi);
    vec2 state = vec2(1.0 / r0, -v_r / (r0 * v_phi));

    float phi = 0.0;
    float u_min = state.x;
    bool absorbed = false;
    int crossings = 0;
    vec3 disk_accum = vec3(0.0);
    float prev_yf = e_r.y;

    for (int step = 0; step < MAX_RK4_STEPS; ++step) {
        if (state.x * rs >= 1.0) {
            absorbed = true;
            break;
        }
        if (state.x * rs < ESCAPE_U_FACTOR && state.y < 0.0) break;
        float h = DPHI_BASE / max(state.x * rs * 5.0, 1.0);
        h *= random_range(1.0 - DPHI_JITTER, 1.0 + DPHI_JITTER);

        vec2 next = rk4_step(state, h, rs);
        float np = phi + h;
        float nyf = cos(np) * e_r.y + sin(np) * e_phi.y;

        if (prev_yf * nyf < 0.0) {
            float frac = prev_yf / (prev_yf - nyf);
            float pc = phi + frac * h;
            float uc = mix(state.x, next.x, frac);
            float r_rs = (1.0 / uc) / rs;
            if (r_rs > DISK_INNER_RS && r_rs < DISK_OUTER_RS) {
                vec3 pos_rel = (1.0 / uc) * (cos(pc) * e_r + sin(pc) * e_phi);
                float ang = atan(pos_rel.z, pos_rel.x);
                float edge = smoothstep(0.0, 0.6, r_rs - DISK_INNER_RS) *
                        smoothstep(0.0, 1.0, DISK_OUTER_RS - r_rs);
                float layer = exp(-float(crossings) * 0.6);
                disk_accum += disk_colour(r_rs, ang) * edge * layer;
                crossings++;
            }
        }

        prev_yf = nyf;
        state = next;
        phi = np;
        u_min = max(u_min, state.x);
    }

    vec3 background;
    if (absorbed) {
        background = vec3(0.0);
    } else {
        float r_end = 1.0 / max(state.x, 1e-6);
        float dr_dphi = -state.y / max(state.x * state.x, 1e-6);
        float cp = cos(phi), sp = sin(phi);
        vec3 bent = normalize(dr_dphi * (cp * e_r + sp * e_phi) + r_end * (-sp * e_r + cp * e_phi));
        vec4 clip = ubo.proj * ubo.view * vec4(cam_pos + bent * 1000.0, 1.0);
        if (clip.w <= 0.0) {
            background = vec3(0.0);
        } else {
            vec2 uvo = (clip.xy / clip.w) * 0.5 + 0.5;
            background = (any(lessThan(uvo, vec2(0.0))) || any(greaterThan(uvo, vec2(1.0))))
                ? vec3(0.0) : texture(scene_tex, uvo).rgb;
        }
    }

    float halo = exp(-pow((u_min * rs - 2.0 / 3.0) * 4.5, 2.0));
    halo *= clamp(phi / 3.14159265, 0.0, 1.5);
    vec3 halo_col = vec3(1.05, 0.9, 0.6) * halo * 0.8;

    vec3 c = background + disk_accum + halo_col;
    c = c / (c + vec3(1.0)) * 1.6;
    out_colour = vec4(c, 1.0);
}
