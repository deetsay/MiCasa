#pragma once

#include <filesystem>

namespace fs = std::filesystem;

class Folder {

public:
    fs::path path;

    int pic;

    Folder(fs::path path, int pic);
};
