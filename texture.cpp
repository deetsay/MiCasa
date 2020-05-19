#include "texture.h"

#include <stdio.h>
#include <iostream>

bool Texture::Fit(int *result_w, int *result_h, int limit_w, int limit_h) {

    float scale = ((float) limit_w) / width;
    float scale_h = ((float) limit_h) / height;
    if (scale_h < scale) {
	scale = scale_h;
    }
    if (limit_w==width || limit_h==height) {
	*result_w = (int) width;
	*result_h = (int) height;
	return true;
    } else {
	*result_w = (int) (scale * width);
	*result_h = (int) (scale * height);
	return false;
    }
}

SDL_Surface* Texture::Load_To_Size(const char *filename, int limit_w, int limit_h) {
    if (limit_w < 1 || limit_h < 1) {
	return NULL;
    }

    SDL_Surface *surface = IMG_Load(filename);
    if (surface == NULL) {
	std::cout << "Unable to load image '" << filename << "' ! SDL_image Error: " << IMG_GetError() << std::endl;
	return NULL;
    }
    //std::cout << " format " << SDL_GetPixelFormatName(surface->format->format) << std::endl; 

    float scale = ((float) limit_w) / surface->w;
    float scale_h = ((float) limit_h) / surface->h;
    if (scale_h < scale) {
	scale = scale_h;
    }
    int w = (int)(scale * surface->w);
    int h = (int)(scale * surface->h);

    width = (unsigned short) w;
    height = (unsigned short) h;

    SDL_Rect targetDimensions;
    targetDimensions.x = 0;
    targetDimensions.y = 0;
    targetDimensions.w = w;
    targetDimensions.h = h;

    SDL_Surface *p32BPPSurface = SDL_CreateRGBSurfaceWithFormat(0,
	w,
	h,
	32,
	SDL_PIXELFORMAT_ABGR8888);
    if (p32BPPSurface == NULL) {
	std::cout << "SDL_CreateRGBSurface() failed: " << SDL_GetError() << std::endl;
    }
    //std::cout << " p32bpp format " << SDL_GetPixelFormatName(p32BPPSurface->format->format) << std::endl;
    //SDL_SetAlpha(surface, 0, 255);
    //SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    //SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);

    if (SDL_MUSTLOCK(p32BPPSurface)) {
	SDL_LockSurface(p32BPPSurface);
    }

    if (SDL_BlitScaled(surface, NULL, p32BPPSurface, &targetDimensions) < 0) {
	std::cout << "Error! Did not blit surface: " << SDL_GetError() << std::endl;
    } else {
	SDL_FreeSurface(surface);
	surface = p32BPPSurface;
    }
    if (SDL_MUSTLOCK(p32BPPSurface)) {
	SDL_UnlockSurface(p32BPPSurface);
    }
    return surface;
}

void Texture::InitFromPixels(void *pixels, int mode, int in_w, int in_h) {
    width = (unsigned short) in_w;
    height = (unsigned short) in_h;
    if (in_w < 1 || in_h < 1) {
	texture = 0;
	return;
    }

    glGenTextures(1, &texture);
    if (texture == 0) {
	return;
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, mode, in_w, in_h, 0, mode, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::Texture(void *pixels, int mode, int in_w, int in_h) {
    InitFromPixels(pixels, mode, in_w, in_h);
}

void Texture::Update(void *pixels, int mode) {
    if (texture != 0) {
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (int) width, (int) height, mode, GL_UNSIGNED_BYTE, pixels);
    }
}

void Texture::SurfaceToTexture(SDL_Surface *surface) {
    int mode = GL_RGB;
    switch (surface->format->format) {
	case SDL_PIXELFORMAT_ABGR8888:
	    mode = GL_RGBA;
	    break;

	case SDL_PIXELFORMAT_RGBA8888:
	    mode = GL_BGRA;
	    break;

	case SDL_PIXELFORMAT_RGB24:
	case SDL_PIXELFORMAT_BGR888:
	    mode = GL_RGB;
	    break;

	case SDL_PIXELFORMAT_RGB888:
	    mode = GL_BGR;
	    break;

	default:
	    //std::cout << "Error, image is not truecolor." << std::endl;
	    SDL_FreeSurface(surface);
	    texture = 0;
	    return;
    }
    InitFromPixels(surface->pixels, mode, surface->w, surface->h);
    SDL_FreeSurface(surface);
}

Texture::Texture(const char *filename) {
    SDL_Surface *surface = IMG_Load(filename);
    if (surface == NULL) {
	std::cout << "Unable to load image '" << filename << "' ! SDL_image Error: " << IMG_GetError() << std::endl;
	texture = 0;
	return;
    }
    SurfaceToTexture(surface);
}

Texture::Texture(const char *filename, int limit_w, int limit_h) {
    SDL_Surface *surface = Load_To_Size(filename, limit_w, limit_h);
    if (surface == NULL) {
	std::cout << "Unable to load image '" << filename << "' ! SDL_image Error: " << IMG_GetError() << std::endl;
	texture = 0;
	return;
    }
    SurfaceToTexture(surface);
}

Texture::Texture(const void *data, int size) {
    width = 0;
    height = 0;
    SDL_RWops *rw = SDL_RWFromConstMem(data, size);
    if (rw != NULL) {
	SDL_Surface *surface = IMG_LoadPNG_RW(rw);
	if (surface != NULL) {
	    SurfaceToTexture(surface);
	} else {
	    texture = 0;
	    std::cout << "Unable to load image! SDL_image Error: " << IMG_GetError() << std::endl;
	}
	SDL_RWclose(rw);
    }
}

Texture::~Texture() {
    if (texture != 0) {
	glDeleteTextures(1, &texture);
    }
}
