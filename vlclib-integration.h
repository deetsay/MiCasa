#pragma once

#include "SDL.h"
#include "SDL_mutex.h"

#include "vlc/vlc.h"

#define WIDTH 640
#define HEIGHT 480

#define VIDEOWIDTH 320
#define VIDEOHEIGHT 240

struct context {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_mutex *mutex;
    int n;
};

void *lock(void *data, void **p_pixels);

void unlock(void *data, void *id, void *const *p_pixels);

void display(void *data, void *id);

