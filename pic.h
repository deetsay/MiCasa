#pragma once

#include <string>
#include <filesystem>

#define PIC_ROTATE 3
#define PIC_ROTATE_1 1
#define PIC_ROTATE_2 2
#define PIC_ROTATE_3 3
#define PIC_VIDEO 1<<2
#define PIC_HIDDEN 1<<3

namespace fs = std::filesystem;

class Pic {
private:
    int flags;

public:
    fs::path path;

    Pic(fs::path path, int flags);

    int get_rotation();

    bool is_video();
};
