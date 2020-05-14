#pragma once

#include "SDL.h"
#include <mutex>
#include "SDL_mutex.h"
#include <SDL_opengl.h>

#include "vlc/vlc.h"

#include "folders.h"

class VLCLibIntegration {
private:
    libvlc_instance_t *libvlc;
    libvlc_media_player_t *mp;

public:
    std::mutex mutex;
    char *pixels;

    VLCLibIntegration();
    ~VLCLibIntegration();

    void integrate(Pic *pic);
    void bifurcate();
};

void *vlc_lock(void *data, void **p_pixels);

void vlc_unlock(void *data, void *id, void *const *p_pixels);

//void vlc_display(void *data, void *id);

