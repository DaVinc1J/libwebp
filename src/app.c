#include "headers/app.h"

void app_init(_app *p_app) {
	p_app->config.title = "davincij";
	p_app->config.width = 800;
	p_app->config.height = 600;
	p_app->config.vert_shader = "src/shaders/fullscreen.vert.spv";
	p_app->config.frag_shader = "src/shaders/fullscreen.frag.spv";
	p_app->config.print_fps = false;
	p_app->config.border_px = 20.0f;
	p_app->config.title_px = maxf32(55.0f, TITLE_PX_MIN);
	p_app->config.blur = true;

	glm_vec4_copy((vec4){0.0f, 0.0f, 0.0f, 0.3f}, p_app->config.title_colour);
	glm_vec4_copy((vec4){0.0f, 0.0f, 0.0f, 0.2f}, p_app->config.border_colour);
	glm_vec4_copy((vec4){0.0f, 0.0f, 0.0f, 0.1f}, p_app->config.background_colour);

	p_app->sync.frame_index = 0;
}
