#pragma once

#include <stdio.h>
#include <filesystem>
#include <SDL.h>
#include <SDL_opengl.h>

namespace fs = std::filesystem;

class Pic;

class Folder {

private:
    void addChild(Folder *folder);

public:
    fs::path *path;

    Folder *next;
    Folder *firstBorn;

    Pic *firstPic;

    Folder(fs::path path, int limit_w, int limit_h, GLuint placeholder, int placeholder_w, int placeholder_h);
    virtual ~Folder();

    bool anyChildHasPictures();
};

class Pic {

public:
    Pic *next;
    Pic *prev;
    fs::path *path;

    bool loaded;
    bool reallyLoaded;
    bool isVideo;
    bool isPreview; // @deprecated <- check if limit_w matches instead
    int limit_w;
    int limit_h;
    int width;
    int height;
    GLuint texture;
    GLuint placeholder;

    Pic(Pic *pic);
    Pic(Folder *folder, Pic *previous, fs::path path, int limit_w, int limit_h, GLuint placeholder, int placeholder_w, int placeholder_h);
    virtual ~Pic();

    void load();
    void unload();
};
