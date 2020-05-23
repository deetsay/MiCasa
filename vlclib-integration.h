#pragma once

#include <mutex>
#include "vlc/vlc.h"
#include "folder.h"
#include "texture.h"

class VLCLibIntegration {
private:
    libvlc_instance_t *libvlc;
    libvlc_media_player_t *mp;

public:
    int width;
    int height;

    std::mutex mutex;
    char *pixels;

    VLCLibIntegration();
    ~VLCLibIntegration();

    Texture *integrationPreparation(const char *path);
    void integrate(Texture *texture);
    void bifurcate();
};

void *vlc_lock(void *data, void **p_pixels);

void vlc_unlock(void *data, void *id, void *const *p_pixels);

//void vlc_display(void *data, void *id);

