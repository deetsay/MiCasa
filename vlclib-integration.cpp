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
    const int vlc_argc = 1;

    // Initialise libVLC.
    libvlc = libvlc_new(vlc_argc, vlc_argv);
    if (libvlc == NULL) {
	std::cout << "LibVLC initialization failed!" << std::endl;
    }
}

void VLCLibIntegration::integrate(Pic *pic) {
    if (mp == NULL) {
	if (libvlc == NULL) {
	    return;
	}
	std::string path = pic->path->string();
	libvlc_media_t *m = libvlc_media_new_path(libvlc, path.c_str());
	mp = libvlc_media_player_new_from_media(m);
	//libvlc_media_parse_with_options(m, libvlc_media_parse_network, 10);
	//if (libvlc_media_get_parsed_status(m) != libvlc_media_parsed_status_done) {
	//    std::cout << "LibVLC parse error!" << std::endl;
	//}
	libvlc_media_release(m);

	//if (!libvlc_media_is_parsed(m)) {
	//    libvlc_media_parse(m);
	//}
	libvlc_video_get_size(mp, 0, (unsigned int *) (&(pic->width)), (unsigned int *) (&(pic->height)));

	//std::cout << "w = " << currentPic->width << " h = " << currentPic->height << std::endl;

	//pixels = (char *) operator new[](sizeof(char) * pic->width * pic->height * 4, (std::align_val_t)(32));
	//pixels = new (std::align_val_t(32)) char[pic->width * pic->height * 4]();
	pixels = new char[pic->width * pic->height * 4]();
	CreateNewTexture(&(pic->texture), GL_RGBA, pic->width, pic->height, (void *) pixels);
	pic->reallyLoaded = true;

	//std::cout << "texture = " << currentPic->texture << " pixels " << (void *) vlc_context.pixels << std::endl;

	libvlc_video_set_callbacks(mp, vlc_lock, vlc_unlock, NULL, this);
	libvlc_video_set_format(mp, "RGBA", pic->width, pic->height, pic->width*4);
	libvlc_media_player_play(mp);
    }

    mutex.lock();
    UpdateTexture(&(pic->texture), GL_RGBA, pic->width, pic->height, (void *) pixels);
    mutex.unlock();
    //if(!pause) { vlc_context.n++; }
    //SDL_Delay(1000/10);
}

void VLCLibIntegration::bifurcate() {
    if (mp != NULL) {
	libvlc_media_player_stop(mp);
	libvlc_media_player_release(mp);
	mp = NULL;
    }
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
