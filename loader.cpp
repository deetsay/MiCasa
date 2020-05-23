#include "loader.h"

#include <iostream>
#include <chrono>

void Loader::unload(int i) {
    if (photostream->at(i).is_video()) {
	return;
    }
  /*  std::unordered_map<int, Texture*>::iterator pt = previews->find(i);
    if (pt != previews->end()) {
	Texture tex = pt->second;
	tex.Unload();
	previews->erase(i);
    }*/
}

void Loader::try_load(int i, int lw, int lh) {
    if (photostream->at(i).is_video()) {
	return;
    }
    std::string path = photostream->at(i).path;
    std::unordered_map<int, Texture*>::iterator pt = previews->find(i);
    if (pt == previews->end()) {
	Texture *tex = new Texture(path.c_str(), lw, lh);
	previews->try_emplace(i, tex);
	return;
    }
    pt->second->Update(path.c_str(), lw, lh);
}

void Loader::background_load() {
    while (done == false) {
	mutex.lock();
	int midp = fvp+((lvp-fvp)/2);
	int midm = midp-1;
	int lw = limit_w;
	int lh = limit_h;
	reset = false;
	mutex.unlock();

	while (reset == false && done == false) {
	    int pause = 0;
	    if (midm>=0 && midm<photostream->size()) {
		if (fvp-midm < 2*(lvp-fvp)) {
		    try_load(midm, lw, lh);
		} else if (fvp-midm > 4*(lvp-fvp)) {
		    unload(midm);
		}
		midm--;
	    } else {
		pause++;
	    }
	    if (midp>=0 && midp<photostream->size()) {
		if (midp-lvp < 2*(lvp-fvp)) {
		    try_load(midp, lw, lh);
		} else if (midp-lvp > 4*(lvp-fvp)) {
		    unload(midp);
		}
		midp++;
	    } else {
		pause++;
	    }
	    if (pause == 2) {
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	    }
	}
    }
}

void Loader::fyi(int fvp, int lvp, int limit_w, int limit_h) {
    if (fvp != this->fvp || lvp != this->lvp || limit_w != this->limit_w || limit_h != this->limit_h) {
	mutex.lock();
	this->fvp = fvp;
	this->lvp = lvp;
	this->limit_w = limit_w;
	this->limit_h = limit_h;
	this->reset = true;
	mutex.unlock();
    }
    if (thread == NULL) {
	thread = new std::thread(&Loader::background_load, this);
    }
}

Loader::Loader(std::vector<Pic> *photostream, std::unordered_map<int, Texture*> *previews) {
    thread = NULL;
    fvp = -1;
    lvp = -1;
    reset = true;
    done = false;
    this->photostream = photostream;
    this->previews = previews;
}

Loader::~Loader() {
    done = true;
    if (thread != NULL) {
	thread->join();
	delete thread;
    }
}
