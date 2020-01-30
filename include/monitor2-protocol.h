#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <SDL2/SDL.h>
#include <stdint.h>
#include "vec2.h"
#endif

typedef uint64_t button_field;

typedef struct {
    struct {
	button_field pressed;
    }
	buttons;
    struct {
	vec2(int16_t) pos;
    }
	mouse;
}
    button_input;

typedef struct {
    uint32_t keycode;
    button_field field;
}
    bind_button;

typedef struct {
    uint32_t handle;
    uint16_t name_size;
    char name[];
}
    cache_image;

typedef struct {
    uint32_t handle;
    vec2(uint16_t) pos;
}
    set_tile;

typedef struct {
    vec2(uint16_t) size;
}
    new_matrix;

typedef struct {
    vec2(int32_t) pos;
}
    center_view;

typedef struct {
    double scale;
}
    set_scale;

enum monitor_header {
    MON_INPUT,
    MON_BIND,
    MON_UNBIND,
    MON_CACHE,
    MON_SET_TILE,
    MON_OVERLAY_TILE,
    MON_CENTER_VIEW,
    MON_SET_SCALE,
};
