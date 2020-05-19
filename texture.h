#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

class Texture {
private:
    SDL_Surface *Load_To_Size(const char *filename, int limit_w, int limit_h);
    void SurfaceToTexture(SDL_Surface *surface);
    void InitFromPixels(void *pixels, int mode, int in_w, int in_h);

public:
    GLuint texture;
    unsigned short width;
    unsigned short height;

    Texture(const char *filename);
    Texture(const char *filename, int limit_w, int limit_h);
    Texture(const void *data, int size);
    Texture(void *pixels, int mode, int in_w, int in_h);
    virtual ~Texture();

    bool Fit(int *result_w, int *result_h, int limit_w, int limit_h);

    void Update(void *pixels, int mode);
};
