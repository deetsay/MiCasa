#pragma once

#include <filesystem>
#include "texture.h"

namespace fs = std::filesystem;

class Pic {

public:
    fs::path *path;

    bool isVideo;

    Texture *texture;

    Pic(fs::path path);
    virtual ~Pic();

    void load(unsigned short limit_w, unsigned short limit_h);
    void unload();
};

class PicNode {
public:
    Pic *pic;
    PicNode *next;
    PicNode *prev;

    PicNode(Pic *pic);
};
