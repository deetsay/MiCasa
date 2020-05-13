#include <stdlib.h>
#include <assert.h>

#include "vlclib-integration.h"

// VLC prepares to render a video frame.
void *lock(void *data, void **p_pixels) {

    struct context *c = (context *)data;

    int pitch;
    SDL_LockMutex(c->mutex);
    SDL_LockTexture(c->texture, NULL, p_pixels, &pitch);

    return NULL; // Picture identifier, not needed here.
}

// VLC just rendered a video frame.
void unlock(void *data, void *id, void *const *p_pixels) {

    struct context *c = (context *)data;

    //uint16_t *pixels = (uint16_t *)*p_pixels;

    // We can also render stuff.
    /*int x, y;
    for(y = 10; y < 40; y++) {
        for(x = 10; x < 40; x++) {
            if(x < 13 || y < 13 || x > 36 || y > 36) {
                pixels[y * VIDEOWIDTH + x] = 0xffff;
            } else {
                // RV16 = 5+6+5 pixels per color, BGR.
                pixels[y * VIDEOWIDTH + x] = 0x02ff;
            }
        }
    }*/

    SDL_UnlockTexture(c->texture);
    SDL_UnlockMutex(c->mutex);
}

// VLC wants to display a video frame.
void display(void *data, void *id) {

    struct context *c = (context *)data;

    SDL_Rect rect;
    rect.w = VIDEOWIDTH;
    rect.h = VIDEOHEIGHT;
    rect.x = 0;
    rect.y = 0;

    SDL_SetRenderDrawColor(c->renderer, 0, 80, 0, 255);
    SDL_RenderClear(c->renderer);
    SDL_RenderCopy(c->renderer, c->texture, NULL, &rect);
    SDL_RenderPresent(c->renderer);
}
