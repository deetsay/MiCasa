#pragma once

#include <SDL_opengl.h>

bool LoadTextureFromFile(const char *filename, GLuint *texture, int *width, int *height);

bool LoadPreviewTextureFromFile(const char *filename, GLuint *texture, int *width, int *height, int limit_w, int limit_h);

void LoadTextureFromMemory(const void *data, int size, GLuint *texture);

bool Fit2U(int *resultWidth, int *resultHeight, int width, int height, int limit_w, int limit_h, bool stretch);

void CreateNewTexture(GLuint *texture, int mode, int width, int height, void *pixels);

void UpdateTexture(GLuint *texture, int mode, int width, int height, void *pixels);
