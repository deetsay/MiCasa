#pragma once

#include <mutex>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

class Texture {
private:
    GLuint texture;
    SDL_Surface *surface;
    std::mutex mutex;
    int width;
    int height;

    void SurfaceToTexture();
    void InitFromPixels(void *pixels, int mode, int in_w, int in_h);
    void InitFromFile(const char *filename, int limit_w, int limit_h);

public:
    bool size_ok(int limit_w, int limit_h);
    void *Fit(int *out_w, int *out_h, int limit_w=0, int limit_h=0);

    void Update(const char *filename, int limit_w, int limit_h);
    void Update(void *pixels, int mode);

    Texture(const char *filename);
    Texture(const char *filename, int limit_w, int limit_h);
    Texture(const void *data, int size);
    Texture(void *pixels, int mode, int in_w, int in_h);
    virtual ~Texture();
};
