#include "vlclib-integration.h"

#include <iostream>

#include "texture.h"

void *vlc_lock(void *opaq, void **pixels) {
    VLCLibIntegration *vlcinteg = (VLCLibIntegration *) opaq;
    vlcinteg->mutex.lock();
    *pixels = (void *) vlcinteg->pixels;
    return NULL; // Picture identifier, not needed here.
}

void vlc_unlock(void *opaq, void *id, void *const *pixels) {
    VLCLibIntegration *vlcinteg = (VLCLibIntegration *) opaq;
    vlcinteg->mutex.unlock();
}

VLCLibIntegration::VLCLibIntegration() {
    mp = NULL;
    pixels = NULL;
    //started = false;

    // If you don't have this variable set you must have plugins directory
    // with the executable or libvlc_new() will not work!
    //printf("VLC_PLUGIN_PATH=%s\n", getenv("VLC_PLUGIN_PATH"));

    #ifdef __APPLE__
        if (getenv("VLC_PLUGIN_PATH") == NULL) {
	    setenv("VLC_PLUGIN_PATH", "/Applications/VLC.app/Contents/MacOS/plugins", true);
	};
    #endif
    const char *vlc_argv[] = {
	"--no-xlib",
	"-I", "dummy",
	"--ignore-config"
    };
    const int vlc_argc = 4;

    // Initialise libVLC.
    libvlc = libvlc_new(vlc_argc, vlc_argv);
    if (libvlc == NULL) {
	std::cout << "LibVLC initialization failed!" << std::endl;
    }
}

Texture *VLCLibIntegration::integrationPreparation(const char *path) {
    Texture *texture = NULL;
    if (libvlc != NULL && mp == NULL) {
	libvlc_media_t *m = libvlc_media_new_path(libvlc, path);
	mp = libvlc_media_player_new_from_media(m);
	libvlc_media_parse_with_options(m, libvlc_media_parse_network, -1);
	while (libvlc_media_get_parsed_status(m) == 0) {
	}
	if (libvlc_media_get_parsed_status(m) == libvlc_media_parsed_status_done
	    && libvlc_video_get_size(mp, 0, (unsigned int*)(&this->width), (unsigned int*)(&this->height)) == 0) {
	    libvlc_media_release(m);
	    pixels = new char[width * height * 4]();
	    texture = new Texture((void *) pixels, GL_RGBA, width, height);

	    libvlc_video_set_callbacks(mp, vlc_lock, vlc_unlock, NULL, this);
	    libvlc_video_set_format(mp, "RGBA", width, height, width*4);
	    libvlc_media_player_play(mp);
	} else {
	    libvlc_media_release(m);
	    bifurcate();
	}
    }
    return texture;
}

void VLCLibIntegration::integrate(Texture *texture) {
    //if (started == false) {
    if (libvlc == NULL) {
	return;
    }
    //} else {
    if (texture != NULL) {
	mutex.lock();
	texture->Update((void *) pixels, GL_RGBA);
	mutex.unlock();
    }

    //if(!pause) { vlc_context.n++; }
    //SDL_Delay(1000/10);
}

void VLCLibIntegration::bifurcate() {
    if (mp != NULL) {
	libvlc_media_player_stop(mp);
	libvlc_media_player_release(mp);
	mp = NULL;
    }
    //started = false;
    if (pixels != NULL) {
	delete [] pixels;
	pixels = NULL;
    }
}

VLCLibIntegration::~VLCLibIntegration() {
    bifurcate();

    if (libvlc != NULL) {
	libvlc_release(libvlc);
    }
}
