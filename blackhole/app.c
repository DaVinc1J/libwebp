#include "headers/app.h"
#include "headers/maths.h"

void app_init(_app *p_app) {

	p_app->config.win.title = "davincij";
	p_app->config.win.width = 800;
	p_app->config.win.height = 600;
	p_app->config.win.flags = CONFIG_FLAG_NONE;
	p_app->config.win.render_extent_modifier = 1.5f;

	p_app->config.lod.MESH_SPHERE_LOD_SEGMENTS[0] = 16;
	p_app->config.lod.MESH_SPHERE_LOD_SEGMENTS[1] = 32;
	p_app->config.lod.MESH_SPHERE_LOD_SEGMENTS[2] = 64;
	p_app->config.lod.MESH_SPHERE_LOD_SEGMENTS[3] = 128;
	p_app->config.lod.MESH_SPHERE_LOD_RINGS[0] = 16;
	p_app->config.lod.MESH_SPHERE_LOD_RINGS[1] = 32;
	p_app->config.lod.MESH_SPHERE_LOD_RINGS[2] = 64;
	p_app->config.lod.MESH_SPHERE_LOD_RINGS[3] = 128;
	p_app->config.lod.MESH_SPHERE_LOD_DISTANCES[0] = 2500.0f;
	p_app->config.lod.MESH_SPHERE_LOD_DISTANCES[1] = 6400.0f;
	p_app->config.lod.MESH_SPHERE_LOD_DISTANCES[2] = 10000.0f;
	p_app->config.lod.MESH_SPHERE_LOD_RADIUS_MODIFIER = 1.0f;

	p_app->config.grid.range = 0.001f;
	p_app->config.grid.spacing = 0.001f;
	p_app->config.grid.seg_len = 0.001f;
	p_app->config.grid.depth_multiplier = 2.0f;
	p_app->config.grid.softening_multiplier = 0.125f;

	p_app->config.sim.flags = SIM_FLAG_NONE;

	p_app->sync.frame_index = 0;

	p_app->shader.mesh_vert = "src/shaders/mesh.vert.spv";
	p_app->shader.mesh_frag = "src/shaders/mesh.frag.spv";
	p_app->shader.billboard_vert = "src/shaders/billboard.vert.spv";
	p_app->shader.billboard_frag = "src/shaders/billboard.frag.spv";
	p_app->shader.grid_vert = "src/shaders/grid.vert.spv";
	p_app->shader.grid_frag = "src/shaders/grid.frag.spv";
	p_app->shader.lens_vert = "src/shaders/lens.vert.spv";
	p_app->shader.lens_frag = "src/shaders/lens.frag.spv";

	glm_vec3_copy((vec3){10.0f, 10.0f, 10.0f}, p_app->view.camera_pos);
	glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, p_app->view.target);
	glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, p_app->view.world_up);
	p_app->view.fov_y = 90.0f;
	p_app->view.near_plane = 0.01f;
	p_app->view.far_plane = 1000.0f;
	p_app->view.rotation_speed = 0.0f;
	p_app->view.speed = 0.5f;
	p_app->view.lerp_speed = 0.1f;
	p_app->view.sensitivity = 0.5f;
	p_app->view.yaw = -90.0f;
	p_app->view.pitch = 0.0f;
	p_app->view.first_mouse = true;
	p_app->view.mouse_locked = true;

	static _solar_object solar_objects[] = {
		{
    	.position = {-40.0f, 0.0f, 0.0f},
    	.velocity = {0.0f, 0.0f, 1.66f},
			.colour_id = COLOUR_NOT_SET,
			.mass = 1.0e9f,
			.intensity = 1000.0f,
			.type = SOLAR_OBJECT_TYPE_LIGHT_EMIT,
			.planet_type = PLANET_TYPE_WHITE_DWARF
		},
		{
    	.position = {-40.0f, 0.0f, -40.0f},
    	.velocity = {0.0f, 0.0f, 1.66f},
			.colour_id = COLOUR_RED,
			.mass = 1.0e9f,
			.intensity = 1000.0f,
			.type = SOLAR_OBJECT_TYPE_PLAIN,
			.planet_type = PLANET_TYPE_WHITE_DWARF
		},

		{
    	.position = {10.0f, 0.0f, 0.0f},
    	.velocity = {0.0f, 0.0f, -1.66f},
			.colour_id = COLOUR_NOT_SET,
			.mass = 1.0e9f,
			.type = SOLAR_OBJECT_TYPE_BLACKHOLE,
			.schwarzschild_radius = 1.5f,
		},
	};


	p_app->obj.primitive_count = 0;

	p_app->obj.billboards = NULL;
	p_app->obj.billboard_count = 0;
	p_app->obj.billboard_max = 0;

	p_app->obj.solar_object_count = sizeof(solar_objects) / sizeof(solar_objects[0]);

	for (int i = 0; i < p_app->obj.solar_object_count; i++) {
		set_radius(&solar_objects[i]);
		set_colour(&solar_objects[i]);
	}

	p_app->obj.solar_objects = malloc(sizeof(_solar_object) * p_app->obj.solar_object_count);
	memcpy(p_app->obj.solar_objects, solar_objects, sizeof(solar_objects));

	glm_vec4_copy((vec4){1.0f, 1.0f, 1.0f, 0.0f}, p_app->lighting.ambient);
}
