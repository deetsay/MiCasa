#include "folders.h"
#include <filesystem>
//#include <cstddef>
#include <iostream>
#include <regex>
#include <SDL.h>
#include <SDL_opengl.h>
#include "texture.h"

namespace fs = std::filesystem;

const std::regex pic_regex("^[^\\.].*\\.(jpg|jpeg|gif|png|mpg|mp4|avi|3gp)$", std::regex_constants::icase);

const std::regex vid_regex("^[^\\.].*\\.(mpg|mp4|avi|3gp)$", std::regex_constants::icase);

Folders::Folders(char *pathC) {
    this->first = NULL;
    this->last = NULL;
    if (pathC == NULL) {
	new Folder(this, fs::current_path());
    } else {
	new Folder(this, fs::path(pathC));
    }
}

Folders::~Folders() {
    Folder *folder = this->first;
    while (folder != NULL) {
	Folder *next = folder->next;
	delete folder;
	folder = next;
    }
}

Folder::Folder(Folders *folders, fs::path path) {
    this->next = NULL;
    this->prev = folders->last;
    this->path = new fs::path(path);
    if (folders->last == NULL) {
	folders->first = this;
    } else {
	folders->last->next = this;
    }
    folders->last = this;
    this->firstPic = NULL;
    this->lastPic = NULL;

    std::cout << "Folder created for " << path << std::endl;
    this->hasPictures = false;
    this->loaded = false;

    for (const fs::directory_entry &entry : fs::directory_iterator(*this->path)) {
	if (fs::is_regular_file(entry)) {
	    std::smatch pic_match;
	    std::string picname = entry.path().filename();
	    if (std::regex_match(picname, pic_match, pic_regex)) {
		this->hasPictures = true;
		break;
	    }
	}
    }

    for (const fs::directory_entry &entry : fs::directory_iterator(path)) {
	if (fs::is_directory(entry)) {
	    new Folder(folders, entry.path());
    	}
    }
}

Folder::~Folder() {
    unload();
    delete this->path;
}

void Folder::load(int limit_w, int limit_h, GLuint placeholder, int placeholder_w, int placeholder_h) {
    if (this->loaded == true) {
	return;
    }
    for (const fs::directory_entry &entry : fs::directory_iterator(*this->path)) {
	if (fs::is_regular_file(entry)) {
	    std::smatch pic_match;
	    std::string picname = entry.path().filename();
	    if (std::regex_match(picname, pic_match, pic_regex)) {
		new Pic(this, entry.path(), limit_w, limit_h, placeholder, placeholder_w, placeholder_h);
	    }
	}
    }
    this->loaded = true;
}

void Folder::unload() {
    Pic *pic = this->firstPic;
    while (pic != NULL) {
	Pic *next = pic->next;
	delete pic;
	pic = next;
    }
    this->firstPic = NULL;
    this->lastPic = NULL;
    this->loaded = false;
}

Pic::Pic(Pic *pic) {
    this->next = pic->next;
    this->prev = pic->prev;
    this->path = pic->path;
    this->limit_w = 0;
    this->limit_h = 0;
    this->loaded = false;
    this->reallyLoaded = false;
    this->texture = pic->texture;
    this->width = pic->width;
    this->height = pic->height;
    this->isVideo = pic->isVideo;
}

Pic::Pic(Folder *folder, fs::path path, int limit_w, int limit_h, GLuint placeholder, int placeholder_w, int placeholder_h) {
    this->next = NULL;
    this->prev = folder->lastPic;
    this->path = new fs::path(path);
    this->limit_w = limit_w;
    this->limit_h = limit_h;
    if (folder->lastPic == NULL) {
	folder->firstPic = this;
    } else {
	folder->lastPic->next = this;
    }
    folder->lastPic = this;

    this->loaded = false;
    this->reallyLoaded = false;
    this->texture = placeholder;
    this->placeholder = placeholder;
    this->width = placeholder_w;
    this->height = placeholder_h;

    std::smatch vid_match;
    std::string picname = path.filename();
    this->isVideo = std::regex_match(picname, vid_match, vid_regex);
}

Pic::~Pic() {
    if (this->reallyLoaded == true) {
	glDeleteTextures(1, &(this->texture));
    }
}

void Pic::load() {
    if (this->isVideo == false) {
	std::string s = this->path->string();
	this->reallyLoaded = LoadTextureFromFile(s.c_str(), &(this->texture), &(this->width), &(this->height), this->limit_w, this->limit_h, true);
    }
    this->loaded = true;
}

void Pic::unload() {
    this->loaded = false;
    if (this->reallyLoaded == true) {
	glDeleteTextures(1, &(this->texture));
    }
    this->texture = this->placeholder;
}
