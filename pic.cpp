#include "pic.h"
#include <regex>
#include <iostream>
#include <SDL_opengl.h>

const std::regex vid_regex("^[^\\.].*\\.(mpg|mp4|avi|3gp)$", std::regex_constants::icase);

Pic::Pic(fs::path path) {
    this->path = new fs::path(path);

    texture = NULL;

    std::smatch vid_match;
    std::string picname = path.filename().string();
    isVideo = std::regex_match(picname, vid_match, vid_regex);
}

void Pic::unload() {
    if (texture != NULL) {
	delete texture;
	texture = NULL;
    }
}

void Pic::load(unsigned short limit_w, unsigned short limit_h) {
    if (isVideo == false) {
	unload();
	std::string s = path->string();
	texture = new Texture(s.c_str(), limit_w, limit_h);
    }
}

Pic::~Pic() {
    delete path;

    if (texture != NULL) {
	delete texture;
    }
}

PicNode::PicNode(Pic *pic) {
    this->pic = pic;
    next = NULL;
    prev = NULL;
}
