#pragma once

#include <stdio.h>
#include <filesystem>
#include <SDL.h>
#include <SDL_opengl.h>

namespace fs = std::filesystem;

class Folder;
class Pic;

class Folders {

public:
    Folder *first;
    Folder *last;

    Folders(char *pathC);
    virtual ~Folders();
};

class Folder {

public:
    Folder *next;
    Folder *prev;
    fs::path *path;

    bool hasPictures;
    bool loaded;
    Pic *firstPic;
    Pic *lastPic;

    Folder(Folders *folders, fs::path path);
    virtual ~Folder();

    void load(int limit_w, int limit_h, GLuint placeholder, int placeholder_w, int placeholder_h);
    void unload();
};

class Pic {

public:
    Pic *next;
    Pic *prev;
    fs::path *path;

    bool loaded;
    bool reallyLoaded;
    int limit_w;
    int limit_h;
    int width;
    int height;
    bool isVideo;
    GLuint texture;
    GLuint placeholder;

    Pic(Pic *pic);
    Pic(Folder *folder, fs::path path, int limit_w, int limit_h, GLuint placeholder, int placeholder_w, int placeholder_h);
    virtual ~Pic();

    void load();
    void unload();
};
