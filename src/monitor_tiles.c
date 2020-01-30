#include <SDL2/SDL.h>

#include "vec2.h"

#include "array.h"

typedef struct {
    SDL_Texture * render;
    array(SDL_Texture*) list;
}
    draw_tile;

typedef struct {
    vec2(uint16_t) size;
    draw_tile ** row;
}
    tile_db;


typedef struct {
    unsigned int last_used_tick;
    SDL_Texture * render;
    vec2(uint16_t) size;
}
    draw_grid;

