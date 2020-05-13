#include <stdio.h>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

SDL_Surface *IMG_Load_To_Size(const char *filename, int *width, int *height, int limit_w, int limit_h, bool dropShadow) {
    SDL_Surface *surface = IMG_Load(filename);
    if (surface == NULL) {
	std::cout << "Unable to load image '" << filename << "' ! SDL_image Error: " << IMG_GetError() << std::endl;
	return NULL;
    }
    //std::cout << " w " << surface->w << " h " << surface->h << " limit " << limit << std::endl;
    //std::cout << " format " << SDL_GetPixelFormatName(surface->format->format) << std::endl; 

    *width = surface->w;
    *height = surface->h;
    if (limit_w <= 4 || limit_h <= 4) {
	return surface;
    }
    if (dropShadow) {
	limit_w = limit_w - 4;
	limit_h = limit_h - 4;
    }
    if ((surface->w > limit_w) || (surface->h > limit_h)) {
	/*SDL_Rect sourceDimensions;
	sourceDimensions.x = 0;
	sourceDimensions.y = 0;
	sourceDimensions.w = surface->w;
	sourceDimensions.h = surface->h;
	*/
	float scale = (float)limit_w / (float)(surface->w);
	float scaleH = (float)limit_h / (float)(surface->h);

	if (scaleH < scale) {
	    scale = scaleH;
	}

	SDL_Rect targetDimensions;
	targetDimensions.x = 0;
	targetDimensions.y = 0;
	targetDimensions.w = (int)(surface->w * scale);
	targetDimensions.h = (int)(surface->h * scale);

	// create a 32 bits per pixel surface to Blit the image to first, before BlitScaled
	// https://stackoverflow.com/questions/33850453/sdl2-blit-scaled-from-a-palettized-8bpp-surface-gives-error-blit-combination/33944312
	SDL_Surface *p32BPPSurface = SDL_CreateRGBSurfaceWithFormat(0,
	    targetDimensions.w + (dropShadow==true ? 4 : 0),
	    targetDimensions.h + (dropShadow==true ? 4 : 0),
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
	    // create another 32 bits per pixel surface are the desired scale
	    /*SDL_Surface *pScaleSurface = SDL_CreateRGBSurface(
		0,
		targetDimensions.w,
		targetDimensions.h,
		32,
		surface->format->Rmask,
		surface->format->Gmask,
		surface->format->Bmask,
		surface->format->Amask);
	    if (pScaleSurface == NULL) {
		printf("SDL_CreateRGBSurface() failed: %s", SDL_GetError());
		//exit(1);
	    }
	    std::cout << " pScale format " << SDL_GetPixelFormatName(pScaleSurface->format->format) << std::endl; 
	    SDL_SetSurfaceBlendMode(pScaleSurface, SDL_BLENDMODE_NONE);

	    // 32 bit per pixel surfaces (loaded from the original file) won't scale down with BlitScaled, suggestion to first fill the surface
	    // 8 and 24 bit depth pngs did not require this
	    // https://stackoverflow.com/questions/20587999/sdl-blitscaled-doesnt-work
	    //SDL_FillRect(pScaleSurface, &targetDimensions, SDL_MapRGBA(pScaleSurface->format, 255, 0, 0, 255));
	    if (SDL_BlitScaled(p32BPPSurface, NULL, pScaleSurface, NULL) < 0) {
	    //if (SDL_BlitScaled(surface, NULL, pScaleSurface, NULL) < 0) {
		printf("Error did not scale surface: %s\n", SDL_GetError());
		SDL_FreeSurface(pScaleSurface);
		pScaleSurface = NULL;
	    } else {
*/
		SDL_FreeSurface(surface);

		//SDL_FreeSurface(pScaleSurface);
		surface = p32BPPSurface;
		//surface = pScaleSurface;
		*width = surface->w;
		*height = surface->h;
	   // }
	}
	if (SDL_MUSTLOCK(p32BPPSurface)) {
	    SDL_UnlockSurface(p32BPPSurface);
	}
	//SDL_FreeSurface(p32BPPSurface);
	//p32BPPSurface = NULL;
    }
    return surface;
}

bool SurfaceToTexture(SDL_Surface *surface, GLuint *texture) {
    int Mode = GL_RGB;
    switch (surface->format->format) {
	case SDL_PIXELFORMAT_ABGR8888:
	    Mode = GL_RGBA;
	    break;

	case SDL_PIXELFORMAT_RGBA8888:
	    Mode = GL_BGRA;
	    break;

	case SDL_PIXELFORMAT_RGB24:
	case SDL_PIXELFORMAT_BGR888:
	    Mode = GL_RGB;
	    break;

	case SDL_PIXELFORMAT_RGB888:
	    Mode = GL_BGR;
	    break;

	default:
	    //std::cout << "Error, image is not truecolor." << std::endl;
	    SDL_FreeSurface(surface);
	    return false;
    }

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, surface->w, surface->h, 0, Mode, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //std::cout << "Image loaded: '" << filename << "' " << std::endl;

    //*width = surface->w;
    //*height = surface->h;
    SDL_FreeSurface(surface);
    return true;
}

bool LoadTextureFromFile(const char *filename, GLuint *texture, int *width, int *height, int limit_w, int limit_h, bool dropShadow) {

    SDL_Surface *surface = IMG_Load_To_Size(filename, width, height, limit_w, limit_h, dropShadow);
    if (surface == NULL) {
	std::cout << "Unable to load image '" << filename << "' ! SDL_image Error: " << IMG_GetError() << std::endl;
	return false;
    }
    return SurfaceToTexture(surface, texture);

    /*if (*width < limit_w && *height < limit_h) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }*/
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
