#include "pic.h"

#include <regex>
#include <iostream>

const std::regex vid_regex("^[^\\.].*\\.(mpg|mp4|avi|3gp)$", std::regex_constants::icase);

Pic::Pic(fs::path path, int rotation) {
    this->path = path;
    this->flags = rotation&PIC_ROTATE;

    std::string filename = path.filename();
    std::smatch vid_match;
    if (std::regex_match(filename, vid_match, vid_regex)) {
	this->flags |= PIC_VIDEO;
    }
}

int Pic::get_rotation() {
    return flags&PIC_ROTATE;
}

bool Pic::is_video() {
    return (flags&PIC_VIDEO)!=0;
}
