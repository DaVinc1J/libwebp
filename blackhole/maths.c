#include "headers/define.h"
#include "headers/maths.h"

float planet_density(_planet_type type) {
    switch (type) {
        case PLANET_TYPE_ROCKY:       return 5500.0f;
        case PLANET_TYPE_ICY:         return 1800.0f;
        case PLANET_TYPE_GAS_GIANT:   return 1000.0f;
        case PLANET_TYPE_ICE_GIANT:   return 1500.0f;
        case PLANET_TYPE_STAR:        return 1400.0f;
        case PLANET_TYPE_WHITE_DWARF: return 1.0e6f;
        default:                      return 1000.0f;
    }
}

u32 planet_colour(_planet_type type) {
    switch (type) {
        case PLANET_TYPE_ROCKY:       return COLOUR_ROCKY;
        case PLANET_TYPE_ICY:         return COLOUR_ICY;
        case PLANET_TYPE_GAS_GIANT:   return COLOUR_GAS_GIANT;
        case PLANET_TYPE_ICE_GIANT:   return COLOUR_ICE_GIANT;
        case PLANET_TYPE_STAR:        return COLOUR_STAR;
        case PLANET_TYPE_WHITE_DWARF: return COLOUR_WHITE_DWARF;
        default:                      return COLOUR_WHITE;
    }
}

void set_radius(_solar_object *obj) {
  if (obj->type == SOLAR_OBJECT_TYPE_BLACKHOLE) {
    obj->radius = obj->schwarzschild_radius;
    return;
  }
  float volume = obj->mass / planet_density(obj->planet_type);
  obj->radius = cbrtf(3.0f * volume / (4.0f * (float)M_PI));
}

void set_colour(_solar_object *obj) {
  if (obj->type == SOLAR_OBJECT_TYPE_BLACKHOLE) {
    obj->colour_id = 0x000000;
    return;
  }
	if (obj->colour_id == COLOUR_NOT_SET) {
		obj->colour_id = planet_colour(obj->planet_type);
	}
}
