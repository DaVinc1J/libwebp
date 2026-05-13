#include "headers/object.h"

void generate_sphere(_app *p_app, u32 segments, u32 rings, _vertex **out_vertices, u32 *out_vcount, u32 **out_indices, u32 *out_icount) {
	u32 vcount = (rings + 1) * (segments + 1);
	u32 icount = rings * segments * 6;

	_vertex *verts = malloc(sizeof(_vertex) * vcount);
	u32 *inds = malloc(sizeof(u32) * icount);

	u32 v = 0;
	for (u32 y = 0; y <= rings; y++) {
		float vcoord = (float)y / rings;
		float phi = vcoord * M_PI;

		for (u32 x = 0; x <= segments; x++) {
			float ucoord = (float)x / segments;
			float theta = ucoord * 2.0f * M_PI;

			float sx = sinf(phi) * cosf(theta);
			float sy = cosf(phi);
			float sz = sinf(phi) * sinf(theta);

			verts[v].pos[0] = sx * p_app->config.lod.MESH_SPHERE_LOD_RADIUS_MODIFIER;
			verts[v].pos[1] = sy * p_app->config.lod.MESH_SPHERE_LOD_RADIUS_MODIFIER;
			verts[v].pos[2] = sz * p_app->config.lod.MESH_SPHERE_LOD_RADIUS_MODIFIER;

			verts[v].norm[0] = sx;
			verts[v].norm[1] = sy;
			verts[v].norm[2] = sz;

			verts[v].uv[0] = ucoord;
			verts[v].uv[1] = vcoord;

			verts[v].data[0] = 0xFFFFFF;
			verts[v].data[1] = 0;
			verts[v].data[2] = 0;
			verts[v].data[3] = 0;

			v++;
		}
	}

	u32 i = 0;
	for (u32 y = 0; y < rings; y++) {
		for (u32 x = 0; x < segments; x++) {
			u32 i0 = y * (segments + 1) + x;
			u32 i1 = i0 + 1;
			u32 i2 = i0 + (segments + 1);
			u32 i3 = i2 + 1;

			inds[i++] = i0;
			inds[i++] = i1;
			inds[i++] = i2;

			inds[i++] = i1;
			inds[i++] = i3;
			inds[i++] = i2;
		}
	}

	*out_vertices = verts;
	*out_vcount = vcount;
	*out_indices = inds;
	*out_icount = icount;
}

void create_spheres(_app *p_app) {
	u32 sphere_lods = MESH_SPHERE_LOD_COUNT;
	u32 base = MESH_SHAPE_SPHERE_LOD0;

	p_app->mesh.vertex_counts = malloc(sizeof(u32) * MESH_SPHERE_LOD_COUNT);
	p_app->mesh.index_counts = malloc(sizeof(u32) * MESH_SPHERE_LOD_COUNT);

	p_app->mesh.vertices = malloc(sizeof(_vertex*) * MESH_SPHERE_LOD_COUNT);
	p_app->mesh.indices = malloc(sizeof(u32*) * MESH_SPHERE_LOD_COUNT);
	p_app->mesh.vertex_allocations = malloc(sizeof(VmaAllocation) * MESH_SPHERE_LOD_COUNT);
	p_app->mesh.index_allocations = malloc(sizeof(VmaAllocation) * MESH_SPHERE_LOD_COUNT);
	p_app->mesh.vertex_buffers = malloc(sizeof(VkBuffer) * MESH_SPHERE_LOD_COUNT);
	p_app->mesh.index_buffers = malloc(sizeof(VkBuffer) * MESH_SPHERE_LOD_COUNT);

	for (u32 lod = 0; lod < sphere_lods; lod++) {
		_vertex *verts;
		u32 *inds;
		u32 vcount, icount;

		generate_sphere(
			p_app,
			p_app->config.lod.MESH_SPHERE_LOD_SEGMENTS[lod],
			p_app->config.lod.MESH_SPHERE_LOD_RINGS[lod],
			&verts,
			&vcount,
			&inds,
			&icount
		);

		u32 id = base + lod;

		p_app->mesh.vertex_counts[id] = vcount;
		p_app->mesh.index_counts[id]  = icount;

		p_app->mesh.vertices[id] = verts;
		p_app->mesh.indices[id]  = inds;
	}
}

_billboard generate_billboard(_solar_object *solar_object) {
	_billboard billboard = {
		.light_pos_w = {
			.position = {
				solar_object->position[0],
				solar_object->position[1],
				solar_object->position[2],
			},
			.intensity = solar_object->intensity,
		},
		.size = {solar_object->radius, solar_object->radius},
		.rotation = {0.0f, 0.0f},
		.light_data = {
			.colour_id = solar_object->colour_id,
			.reserved1 = 0,
			.reserved2 = 0,
			.reserved3 = 0,
		},
		.type = BILLBOARD_TYPE_LIGHT,
		.shape = BILLBOARD_SHAPE_CIRCLE,
		.location = BILLBOARD_LOCATION_IN_WORLD,
		.reserved = 0,
	};

	return billboard;
}

void create_grid_lines(_app *p_app) {

	float range = p_app->config.grid.range;
	float spacing = p_app->config.grid.spacing;
	float seg_len = p_app->config.grid.seg_len;

	i32 lines_per_axis = (i32)(2.0f * range / spacing) + 1;
	i32 segs_per_line = (i32)(2.0f * range / seg_len);
	i32 verts_per_line = segs_per_line * 2;
	p_app->grid.vertex_count = lines_per_axis * 2 * verts_per_line;

	p_app->grid.verts = malloc(sizeof(_grid_vertex) * p_app->grid.vertex_count);
	i32 v = 0;

	for (i32 iz = 0; iz < lines_per_axis; iz++) {
		float z = -range + iz * spacing;
		for (i32 s = 0; s < segs_per_line; s++) {
			float x0 = -range + s * seg_len;
			float x1 = x0 + seg_len;
			p_app->grid.verts[v++] = (_grid_vertex){{ x0, 0.0f, z }};
			p_app->grid.verts[v++] = (_grid_vertex){{ x1, 0.0f, z }};
		}
	}

	for (i32 ix = 0; ix < lines_per_axis; ix++) {
		float x = -range + ix * spacing;
		for (i32 s = 0; s < segs_per_line; s++) {
			float z0 = -range + s * seg_len;
			float z1 = z0 + seg_len;
			p_app->grid.verts[v++] = (_grid_vertex){{ x, 0.0f, z0 }};
			p_app->grid.verts[v++] = (_grid_vertex){{ x, 0.0f, z1 }};
		}
	}
}

static float sample_max_depth(_app *p_app, float gravity_scale, float softening) {
    u32 count = p_app->obj.solar_object_count;
    float max_depth = 0.0f;
    
    for (u32 i = 0; i < count; i++) {
        float xi = p_app->obj.solar_objects[i].position[0];
        float zi = p_app->obj.solar_objects[i].position[2];

        float raw = 0.0f;
        for (u32 j = 0; j < count; j++) {
            float dx = xi - p_app->obj.solar_objects[j].position[0];
            float dz = zi - p_app->obj.solar_objects[j].position[2];
            float r = sqrtf(dx * dx + dz * dz);
            raw -= gravity_scale * p_app->obj.solar_objects[j].mass / (r + softening);
        }

        float compressed = logf(1.0f - raw);
        float depth = compressed * compressed;
        if (depth > max_depth) max_depth = depth;
    }
    
    return max_depth;
}

void compute_grid_params(_app *p_app, float *out_gravity_scale, float *out_softening, float *out_max_depth, float *out_max_radius) {
    u32 count = p_app->obj.solar_object_count;
    if (count == 0) {
        *out_gravity_scale = 1.0f;
        *out_softening = 1.0f;
        *out_max_depth = 1.0f;
        return;
    }

    float min_radius = FLT_MAX;
		float max_radius = FLT_MIN;
    float max_mass = FLT_MIN;
    for (u32 i = 0; i < count; i++) {
        float r = p_app->obj.solar_objects[i].radius;
        float m = p_app->obj.solar_objects[i].mass;
        if (r > 0.0f && r < min_radius) min_radius = r;
				if (r > max_radius) max_radius = r;
        if (m > max_mass) max_mass = m;
    }
    
    float softening = min_radius * p_app->config.grid.softening_multiplier;
		float d_max = max_radius * p_app->config.grid.depth_multiplier;

    float raw_min = 1.0f - expf(sqrtf(d_max));
    float gravity_scale = -raw_min * 2.0f * softening / max_mass;

    for (int iter = 0; iter < 10; iter++) {
        float current_max = sample_max_depth(p_app, gravity_scale, softening);
        float error = current_max - d_max;
        
        if (fabsf(error) < d_max * 0.001f) break;
        
        float h = gravity_scale * 0.001f;
        if (h < 1e-6f) h = 1e-6f;
        float max_plus = sample_max_depth(p_app, gravity_scale + h, softening);
        float derivative = (max_plus - current_max) / h;
        
        if (fabsf(derivative) > 1e-10f) {
            gravity_scale -= error / derivative;
        } else {
            gravity_scale *= (current_max > d_max) ? 0.9f : 1.1f;
        }
        
        if (gravity_scale <= 0.0f) {
            gravity_scale = softening / max_mass * 0.1f;
        }
    }

    float final_max_depth = sample_max_depth(p_app, gravity_scale, softening);
		float final_max_radius = max_radius * 1.01f;

    *out_gravity_scale = gravity_scale;
    *out_softening = softening;
    *out_max_depth = final_max_depth;
		*out_max_radius = final_max_radius;
}

void create_billboards(_app *p_app) {
	u32 billboard_index = 0;
	for (u32 i = 0; i < p_app->obj.solar_object_count; i++) {
		if (p_app->obj.solar_objects[i].type != SOLAR_OBJECT_TYPE_BILLBOARD) continue;
		p_app->obj.solar_objects[i].billboard_index = UINT32_MAX;

		p_app->obj.billboard_count += 1;
		if (p_app->obj.billboard_max <= p_app->obj.billboard_count) {
			p_app->obj.billboards = realloc(p_app->obj.billboards, 2 * sizeof(_billboard) * p_app->obj.billboard_count);
			p_app->obj.billboard_max = 2 * p_app->obj.billboard_count;
		}

		p_app->obj.billboards[billboard_index] = generate_billboard(&p_app->obj.solar_objects[i]);
		p_app->obj.solar_objects[i].billboard_index = billboard_index;
		billboard_index++;
	}
}

void calculate_gravity(_app *p_app) {
	if (!(p_app->config.sim.flags & SIM_FLAG_CALCULATE_GRAVITY)) {
      return;
  }
	const float G_scaled = 6.67430e-11f * MASS_SCALE / (POSITION_SCALE * POSITION_SCALE * POSITION_SCALE);

	for (u32 i = 0; i < p_app->obj.solar_object_count; i++) {
		glm_vec3_zero(p_app->obj.solar_objects[i].acceleration);
	}

	for (u32 i = 0; i < p_app->obj.solar_object_count; i++) {
		_solar_object *obj1 = &p_app->obj.solar_objects[i];

		for (u32 j = i + 1; j < p_app->obj.solar_object_count; j++) {
			_solar_object *obj2 = &p_app->obj.solar_objects[j];

			vec3 r_vec;
			glm_vec3_sub(obj2->position, obj1->position, r_vec);
			float distance = glm_vec3_norm(r_vec);

			if (distance < 1.0f) continue;

			float force_magnitude = G_scaled * obj1->mass * obj2->mass / (distance * distance);

			vec3 force_dir;
			glm_vec3_normalize_to(r_vec, force_dir);

			vec3 force_on_obj1, force_on_obj2;
			glm_vec3_scale(force_dir, force_magnitude, force_on_obj1);
			glm_vec3_scale(force_dir, -force_magnitude, force_on_obj2);

			vec3 accel1, accel2;
			glm_vec3_scale(force_on_obj1, 1.0f / obj1->mass, accel1);
			glm_vec3_scale(force_on_obj2, 1.0f / obj2->mass, accel2);

			glm_vec3_add(obj1->acceleration, accel1, obj1->acceleration);
			glm_vec3_add(obj2->acceleration, accel2, obj2->acceleration);
		}
	}

	for (u32 i = 0; i < p_app->obj.solar_object_count; i++) {
		_solar_object *obj = &p_app->obj.solar_objects[i];

		vec3 velocity_delta;
		glm_vec3_scale(obj->acceleration, p_app->perf.delta_time, velocity_delta);
		glm_vec3_add(obj->velocity, velocity_delta, obj->velocity);

		vec3 position_delta;
		glm_vec3_scale(obj->velocity, p_app->perf.delta_time, position_delta);
		glm_vec3_add(obj->position, position_delta, obj->position);
	}
}

void update_billboard_positions(_app *p_app) {
	if (p_app->obj.billboard_count > 0) {
		for (u32 i = 0; i < p_app->obj.solar_object_count; i++) {
			_solar_object *solar_obj = &p_app->obj.solar_objects[i];
			u32 b = solar_obj->billboard_index;

			if (b < p_app->obj.billboard_count) {
				p_app->obj.billboards[b].light_pos_w.position[0] = solar_obj->position[0];
				p_app->obj.billboards[b].light_pos_w.position[1] = solar_obj->position[1];
				p_app->obj.billboards[b].light_pos_w.position[2] = solar_obj->position[2];
			}
		}
	}
}
