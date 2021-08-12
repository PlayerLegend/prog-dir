#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#define FLAT_INCLUDES


typedef struct image image;
struct image
{
    int width;
    int height;

    const char * id;

    SDL_Texture * image;
};

typedef struct ui_reader_state ui_reader_state;
struct ui_reader_state
{
    int scroll;
};

