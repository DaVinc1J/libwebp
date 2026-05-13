#ifndef LENS_H
#define LENS_H

#include "define.h"


void create_lens_render_pass(_app *p_app);
void create_lens_image(_app *p_app);
void create_lens_framebuffer(_app *p_app);
void create_lens_sampler(_app *p_app);
void create_lens_descriptor_set_layout(_app *p_app);
void create_lens_pipeline(_app *p_app);
void create_lens_descriptor_pool(_app *p_app);
void create_lens_descriptor_sets(_app *p_app);

void cleanup_lens_swapchain(_app *p_app);
void recreate_lens_swapchain(_app *p_app);

#endif
