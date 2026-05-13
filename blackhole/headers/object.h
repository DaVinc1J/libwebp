#ifndef OBJECT_H
#define OBJECT_H

#include "define.h"

void generate_sphere(_app *p_app, u32 segments, u32 rings, _vertex **out_vertices, u32 *out_vcount, u32 **out_indices, u32 *out_icount);
void create_spheres(_app *p_app);
_billboard generate_billboard(_solar_object *solar_object);
void create_billboards(_app *p_app);
void create_grid_lines(_app *p_app);
void calculate_gravity(_app *p_app);
void update_billboard_positions(_app *p_app);
void compute_grid_params(_app *p_app, float *out_gravity_scale, float *out_softening, float *out_max_depth, float *out_max_radius);

#endif
