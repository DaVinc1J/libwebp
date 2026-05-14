#include "headers/app.h"

void app_init(_app *p_app) {
	p_app->config.title = "davincij";
	p_app->config.width = 800;
	p_app->config.height = 600;
	p_app->config.vert_shader = "src/shaders/fullscreen.vert.spv";
	p_app->config.frag_shader = "src/shaders/fullscreen.frag.spv";
	p_app->config.print_fps = false;

	p_app->sync.frame_index = 0;
}
