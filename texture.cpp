#include <stdio.h>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

bool Fit2U(int *resultWidth, int *resultHeight, int width, int height, int limit_w, int limit_h, bool stretch) {
    if (width <= limit_w && height <= limit_h && stretch == false) {
	*resultWidth = width;
	*resultHeight = height;
    }
    if (limit_w < 0) {
	limit_w = width;
    }
    if (limit_h < 0) {
	limit_h = height;
    }
    float scale = (float)limit_w / width;
    float scale_h = (float)limit_h / height;
    if (scale_h < scale) {
	scale = scale_h;
    }
    if ((scale > 1.0f && stretch == false) || scale == 1.0f) {
	*resultWidth = width;
	*resultHeight = height;
	return false;
    } else {
	*resultWidth = width * scale;
	*resultHeight = height * scale;
	return true;
    }
}

SDL_Surface *Load_To_Size(const char *filename, int *width, int *height, int limit_w, int limit_h) {
    if (limit_w < 5 || limit_h < 5) {
	return NULL;
    }

    SDL_Surface *surface = IMG_Load(filename);
    if (surface == NULL) {
	std::cout << "Unable to load image '" << filename << "' ! SDL_image Error: " << IMG_GetError() << std::endl;
	return NULL;
    }
    //std::cout << " w " << surface->w << " h " << surface->h << " limit " << limit << std::endl;
    //std::cout << " format " << SDL_GetPixelFormatName(surface->format->format) << std::endl; 

    Fit2U(width, height, surface->w, surface->h, limit_w-4, limit_h-4, true);

    SDL_Rect targetDimensions;
    targetDimensions.x = 0;
    targetDimensions.y = 0;
    targetDimensions.w = *width;
    targetDimensions.h = *height;

    SDL_Surface *p32BPPSurface = SDL_CreateRGBSurfaceWithFormat(0,
	*width + 4,
	*height + 4,
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
    targetDimensions.x = 4; targetDimensions.y = 4;
    SDL_FillRect(p32BPPSurface, &targetDimensions, SDL_MapRGBA(p32BPPSurface->format, 224, 224, 224, 255));
    targetDimensions.x = 3; targetDimensions.y = 3;
    SDL_FillRect(p32BPPSurface, &targetDimensions, SDL_MapRGBA(p32BPPSurface->format, 192, 192, 192, 255));
    targetDimensions.x = 2; targetDimensions.y = 2;
    SDL_FillRect(p32BPPSurface, &targetDimensions, SDL_MapRGBA(p32BPPSurface->format, 160, 160, 160, 255));
    targetDimensions.x = 1; targetDimensions.y = 1;
    SDL_FillRect(p32BPPSurface, &targetDimensions, SDL_MapRGBA(p32BPPSurface->format, 128, 128, 128, 255));

    targetDimensions.x = 0; targetDimensions.y = 0;
    if (SDL_BlitScaled(surface, NULL, p32BPPSurface, &targetDimensions) < 0) {
	std::cout << "Error! Did not blit surface: " << SDL_GetError() << std::endl;
    } else {
	SDL_FreeSurface(surface);
	surface = p32BPPSurface;
	*width = surface->w;
	*height = surface->h;
    }
    if (SDL_MUSTLOCK(p32BPPSurface)) {
	SDL_UnlockSurface(p32BPPSurface);
    }
    return surface;
}

void CreateNewTexture(GLuint *texture, int mode, int width, int height, void *pixels) {
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void UpdateTexture(GLuint *texture, int mode, int width, int height, void *pixels) {
    glBindTexture(GL_TEXTURE_2D, *texture);
    //glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0,  mode, width, height, 0, mode, GL_UNSIGNED_BYTE, pixels);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, mode, GL_UNSIGNED_BYTE, pixels);
}

bool SurfaceToTexture(SDL_Surface *surface, GLuint *texture) {
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
	    return false;
    }
    CreateNewTexture(texture, mode, surface->w, surface->h, surface->pixels);
    SDL_FreeSurface(surface);
    return true;
}

bool LoadTextureFromFile(const char *filename, GLuint *texture, int *width, int *height) {
    SDL_Surface *surface = IMG_Load(filename);
    if (surface == NULL) {
	std::cout << "Unable to load image '" << filename << "' ! SDL_image Error: " << IMG_GetError() << std::endl;
	return NULL;
    }
    return SurfaceToTexture(surface, texture);
}

bool LoadPreviewTextureFromFile(const char *filename, GLuint *texture, int *width, int *height, int limit_w, int limit_h) {
    SDL_Surface *surface = Load_To_Size(filename, width, height, limit_w, limit_h);
    if (surface == NULL) {
	std::cout << "Unable to load image '" << filename << "' ! SDL_image Error: " << IMG_GetError() << std::endl;
	return false;
    }
    return SurfaceToTexture(surface, texture);
}

void LoadTextureFromMemory(const void *data, int size, GLuint *texture) {
    SDL_RWops *rw = SDL_RWFromConstMem(data, size);
    if (rw != NULL) {
	SDL_Surface *surface = IMG_LoadPNG_RW(rw);
	if (surface != NULL) {
	    SurfaceToTexture(surface, texture);
	} else {
	    std::cout << "Unable to load image! SDL_image Error: " << IMG_GetError() << std::endl;
	}
	SDL_RWclose(rw);
    }
}
