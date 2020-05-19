#pragma once

#include <filesystem>
#include <forward_list>
#include "texture.h"
#include "pic.h"

namespace fs = std::filesystem;

class Folder {

public:
    fs::path *path;

    std::forward_list<Folder*> children;

    std::forward_list<Pic*> pictures;

    Folder(fs::path path);
    virtual ~Folder();

    bool anyChildHasPictures();
};
