#ifndef MATHS_H
#define MATHS_H
#include "define.h"

float planet_density(_planet_type type);
u32 planet_colour(_planet_type type);
void set_radius(_solar_object *obj);
void set_colour(_solar_object *obj);

#endif
