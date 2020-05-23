#include "texture.h"

#include <stdio.h>
#include <iostream>

void *Texture::Fit(int *out_w, int *out_h, int limit_w, int limit_h) {
    mutex.lock();
    if (texture == 0 && surface != NULL) {
	SurfaceToTexture();
    }
    if (limit_w < 1 && limit_h < 1) {
	*out_w = width;
	*out_h = height;
    } else {
	if (limit_w < 1) {
	    limit_w = width;
	}
	if (limit_h < 1) {
	    limit_h = height;
	}
	if (width <= limit_w && height <= limit_h && (limit_w==width || limit_h==height)) {
	    *out_w = width;
	    *out_h = height;
	} else {
	    float scale = ((float) limit_w) / width;
	    float scale_h = ((float) limit_h) / height;
	    if (scale_h < scale) {
		scale = scale_h;
	    }
	    *out_w = (int) (scale * width);
	    *out_h = (int) (scale * height);
	}
    }
    mutex.unlock();
    return (void*)(intptr_t)texture;
}

void Texture::InitFromPixels(void *pixels, int mode, int in_w, int in_h) {
    width = in_w;
    height = in_h;
    if (in_w < 1 || in_h < 1) {
	return;
    }

    glGenTextures(1, &texture);
    if (texture == 0) {
	std::cout << "Unable to init texture of size " << in_w << "x" << in_h << std::endl;
	return;
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, mode, in_w, in_h, 0, mode, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::Update(void *pixels, int mode) {
    if (texture != 0) {
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, mode, GL_UNSIGNED_BYTE, pixels);
    }
}

void Texture::Update(const char *filename, int limit_w, int limit_h) {
    mutex.lock();
    if (surface != NULL) {
	SDL_FreeSurface(surface);
	surface = NULL;
    }
    if (width <= limit_w && height <= limit_h && (limit_w==width || limit_h==height)) {
	mutex.unlock();
	return;
    }

    InitFromFile(filename, limit_w, limit_h);
    texture = 0;
    mutex.unlock();
}

void Texture::SurfaceToTexture() {
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
	    std::cout << "Error, image is not truecolor." << std::endl;
	    SDL_FreeSurface(surface);
	    surface = NULL;
	    return;
    }
    InitFromPixels(surface->pixels, mode, surface->w, surface->h);
    SDL_FreeSurface(surface);
    surface = NULL;
}

Texture::Texture(void *pixels, int mode, int in_w, int in_h) {
    surface = NULL;
    texture = 0;
    InitFromPixels(pixels, mode, in_w, in_h);
}

Texture::Texture(const char *filename) {
    texture = 0;
    surface = IMG_Load(filename);
    if (surface == NULL) {
	std::cout << "Unable to load image '" << filename << "' ! SDL_image Error: " << IMG_GetError() << std::endl;
	return;
    }
}

void Texture::InitFromFile(const char *filename, int limit_w, int limit_h) {
    if (limit_w < 1 || limit_h < 1) {
	return;
    }

    SDL_Surface *load_surface = IMG_Load(filename);
    if (load_surface == NULL) {
	std::cout << "Unable to load image '" << filename << "' ! SDL_image Error: " << IMG_GetError() << std::endl;
	return;
    }
    //std::cout << " format " << SDL_GetPixelFormatName(surface->format->format) << std::endl; 

    float scale = ((float) limit_w) / load_surface->w;
    float scale_h = ((float) limit_h) / load_surface->h;
    if (scale_h < scale) {
	scale = scale_h;
    }

    width = (int)(scale * load_surface->w);
    height = (int)(scale * load_surface->h);

    SDL_Rect targetDimensions;
    targetDimensions.x = 0;
    targetDimensions.y = 0;
    targetDimensions.w = width;
    targetDimensions.h = height;

    surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ABGR8888);
    if (surface != NULL) {
	//std::cout << " format " << SDL_GetPixelFormatName(surface->format->format) << std::endl;
	//SDL_SetAlpha(surface, 0, 255);
	//SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
	//SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
	if (SDL_MUSTLOCK(surface)) {
	    SDL_LockSurface(surface);
	}
	if (SDL_BlitScaled(load_surface, NULL, surface, &targetDimensions) < 0) {
	    std::cout << "Error! Did not blit surface: " << SDL_GetError() << std::endl;
	}
	if (SDL_MUSTLOCK(surface)) {
	    SDL_UnlockSurface(surface);
	}
    } else {
	std::cout << "SDL_CreateRGBSurface() failed: " << SDL_GetError() << std::endl;
    }
    SDL_FreeSurface(load_surface);
}


Texture::Texture(const char *filename, int limit_w, int limit_h) {
    texture = 0;
    surface = NULL;
    InitFromFile(filename, limit_w, limit_h);
}

Texture::Texture(const void *data, int size) {
    surface = NULL;
    texture = 0;
    width = 0;
    height = 0;
    SDL_RWops *rw = SDL_RWFromConstMem(data, size);
    if (rw != NULL) {
	surface = IMG_LoadPNG_RW(rw);
	if (surface == NULL) {
	    std::cout << "Unable to load image! SDL_image Error: " << IMG_GetError() << std::endl;
	}
	SDL_RWclose(rw);
    }
}

Texture::~Texture() {
    if (texture != 0) {
	glDeleteTextures(1, &texture);
	texture = 0;
    }
    if (surface != NULL) {
	SDL_FreeSurface(surface);
	surface = NULL;
    }
}
