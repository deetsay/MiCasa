#include "folder.h"
#include <iostream>
#include <regex>
#include "texture.h"
#include "pic.h"

namespace fs = std::filesystem;

const std::regex pic_regex("^[^\\.].*\\.(jpg|jpeg|gif|png|mpg|mp4|avi|3gp)$", std::regex_constants::icase);

Folder::Folder(fs::path path) {
    this->path = new fs::path(path);

    for (const fs::directory_entry &entry : fs::directory_iterator(path)) {
	if (fs::is_directory(entry)) {
	    Folder *child = new Folder(entry.path());
	    //std::cout << "Working on " << entry.path() << std::endl;
	    if (child->anyChildHasPictures()) {
		//addChild(child);
		children.push_front(child);
	    } else {
		delete child; // Not everybody deserves to make it to the Valley Beyond
	    }
    	}
	if (fs::is_regular_file(entry)) {
	    std::smatch pic_match;
	    std::string picname = entry.path().filename().string();
	    if (std::regex_match(picname, pic_match, pic_regex)) {
		pictures.push_front(new Pic(entry.path()));
	    }
	}
    }
    children.sort([](Folder *first, Folder *second) -> bool {
	    //std::cout << first->path->filename() << " ~~ " << second->path->filename() << std::endl;
	    return strcmp(first->path->filename().c_str(), second->path->filename().c_str()) >= 0;
	});
    pictures.sort([](Pic *l, Pic *r) -> bool {
	    return strcmp(l->path->filename().c_str(), r->path->filename().c_str()) < 0;
	});
}

Folder::~Folder() {
    delete path;

    for (Pic *pic : pictures ) {
	delete pic;
    }
    for (Folder *folder : children ) {
	delete folder;
    }
}

bool Folder::anyChildHasPictures() {
    if (!pictures.empty()) {
	return true;
    }
    for (Folder *folder : children) {
	if (folder->anyChildHasPictures() == true) {
	    return true;
	}
    }
    return false;
}

