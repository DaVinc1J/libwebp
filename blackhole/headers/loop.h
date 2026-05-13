#ifndef VK_LOOP_H
#define VK_LOOP_H

#include "define.h"

void log_performance(_app *p_app);
void draw_frame(_app *p_app);
float get_delta_time();
void update_uniform_buffer(_app *p_app, u32 current_image);
void update_storage_buffers(_app *p_app, u32 current_image);
void update_billboards (_app *p_app);
void update_view(_app *p_app, float time);

#endif
