#pragma once

#include <SDL_opengl.h>

bool LoadTextureFromFile(const char *filename, GLuint *texture, int *width, int *height, int limit_w, int limit_h, bool dropShadow);

void LoadTextureFromMemory(const void *data, int size, GLuint *texture);
