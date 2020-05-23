#pragma once
#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <forward_list>

#include "pic.h"
#include "folder.h"
#include "texture.h"

class Loader {
private:
    std::thread *thread;
    std::mutex mutex;

    std::vector<Pic> *photostream;

    // Preview textures that have been loaded, key = pic index(!)
    std::unordered_map<int, Texture*> *previews;

    int fvp;
    int lvp;

    int limit_w;
    int limit_h;

    bool reset;
    bool done;

    void unload(int i);
    void try_load(int i, int lw, int lh);

public:

    void background_load();
    void fyi(int fvp, int lvp, int limit_w, int limit_h);

    Loader(std::vector<Pic> *photostream, std::unordered_map<int, Texture*> *previews);
    virtual ~Loader();
};
