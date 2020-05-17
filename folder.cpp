#include "folder.h"
#include <filesystem>
#include <iostream>
#include <regex>
#include <SDL.h>
#include <SDL_opengl.h>
#include "texture.h"

namespace fs = std::filesystem;

const std::regex pic_regex("^[^\\.].*\\.(jpg|jpeg|gif|png|mpg|mp4|avi|3gp)$", std::regex_constants::icase);

const std::regex vid_regex("^[^\\.].*\\.(mpg|mp4|avi|3gp)$", std::regex_constants::icase);

void Folder::addChild(Folder *folder) {
    if (this->firstBorn == NULL) {
	this->firstBorn = folder;
	return;
    }
    if (strcmp(folder->path->filename().c_str(), this->firstBorn->path->filename().c_str()) < 0) {
	folder->next = this->firstBorn;
	this->firstBorn = folder;
    } else {
	Folder *lastChild = this->firstBorn;
	Folder *child = lastChild->next;
	while (child != NULL) {
	    if (strcmp(folder->path->filename().c_str(), child->path->filename().c_str()) < 0) {
		folder->next = child;
		lastChild->next = folder;
		return;
	    }
	    lastChild = child;
	    child = child->next;
	}
	lastChild->next = folder;
    }
}

Folder::Folder(fs::path path, int limit_w, int limit_h, GLuint placeholder, int placeholder_w, int placeholder_h) {
    this->path = new fs::path(path);
    this->next = NULL;
    this->firstBorn = NULL;
    this->firstPic = NULL;

    //std::cout << "Folder created for " << path << std::endl;

    Pic *lastPic = NULL;
    for (const fs::directory_entry &entry : fs::directory_iterator(path)) {
	if (fs::is_directory(entry)) {
	    Folder *child = new Folder(entry.path(), limit_w, limit_h, placeholder, placeholder_w, placeholder_h);
	    if (child->anyChildHasPictures()) {
		addChild(child);
	    } else {
		delete child; // Not everybody deserves to make it to the Valley Beyond
	    }
    	}
	if (fs::is_regular_file(entry)) {
	    std::smatch pic_match;
	    std::string picname = entry.path().filename().string();
	    if (std::regex_match(picname, pic_match, pic_regex)) {
		Pic *pic = new Pic(this, lastPic, entry.path(), limit_w, limit_h, placeholder, placeholder_w, placeholder_h);
		if (lastPic == NULL) {
		    this->firstPic = pic;
		} else {
		    lastPic->next = pic;
		}
		lastPic = pic;
	    }
	}
    }
}

Folder::~Folder() {
    delete this->path;

    Pic *pic = this->firstPic;
    while (pic != NULL) {
	Pic *next = pic->next;
	delete pic;
	pic = next;
    }

    Folder *child = this->firstBorn;
    while (child != NULL) {
	Folder *next = child->next;
	delete child;
	child = next;
    }
}

bool Folder::anyChildHasPictures() {
    if (this->firstPic != NULL) {
	return true;
    }
    Folder *child = this->firstBorn;
    while (child != NULL) {
	if (child->anyChildHasPictures() == true) {
	    return true;
	}
	child = child->next;
    }
    return false;
}

Pic::Pic(Pic *pic) {
    this->path = new fs::path(pic->path->string());
    this->isPreview = false;
    this->next = pic->next;
    this->prev = pic->prev;
    this->loaded = false;
    this->reallyLoaded = false;
    this->texture = pic->texture;
    this->width = pic->width;
    this->height = pic->height;
    this->isVideo = pic->isVideo;
}

Pic::Pic(Folder *folder, Pic *previous, fs::path path, int limit_w, int limit_h, GLuint placeholder, int placeholder_w, int placeholder_h) {
    this->path = new fs::path(path);
    this->isPreview = true;
    this->next = NULL;
    this->prev = previous;
    this->limit_w = limit_w;
    this->limit_h = limit_h;

    this->loaded = false;
    this->reallyLoaded = false;
    this->texture = placeholder;
    this->placeholder = placeholder;
    this->width = placeholder_w;
    this->height = placeholder_h;

    std::smatch vid_match;
    std::string picname = path.filename().string();
    this->isVideo = std::regex_match(picname, vid_match, vid_regex);
}

Pic::~Pic() {
    delete this->path;

    if (this->reallyLoaded == true) {
	glDeleteTextures(1, &(this->texture));
    }
}

void Pic::load() {
    if (this->isVideo == false) {
	std::string s = this->path->string();
	if (this->isPreview == true) {
	    this->reallyLoaded = LoadPreviewTextureFromFile(s.c_str(), &(this->texture), &(this->width), &(this->height), this->limit_w, this->limit_h);
	} else {
	    this->reallyLoaded = LoadTextureFromFile(s.c_str(), &(this->texture), &(this->width), &(this->height));
	}
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
