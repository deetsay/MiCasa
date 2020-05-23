#include "folder.h"
#include <iostream>
#include <regex>

namespace fs = std::filesystem;

Folder::Folder(fs::path path, int pic) {
    this->path = path;
    this->pic = pic;
}
