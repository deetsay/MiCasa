#pragma once

#include <vector>
#include <unordered_map>

#include "texture.h"
#include "folder.h"
#include "vlclib-integration.h"
#include "loader.h"

class MiCasa {
private:

    // All the pics
    std::vector<Pic> photostream;

    // All folders
    std::vector<Folder> folders;

    // Preview textures that have been loaded, key = pic index(!)
    std::unordered_map<int, Texture*> previews;

    // Texture / selected pic when vieweing single picture/video (or NULL):
    int chosen_pic;
    Texture *texture;

    int chosen_folder;
    bool folderClicked;

    // VLC stuff all compartmentalized:
    VLCLibIntegration *vlc;

    // Background loader
    Loader *loader;

    // And the rest is all "UI bloat" ;-)
    bool frist;

    // Preview pic max sizes:
    int limit_w;
    int limit_h;

    // Zoom parameters for single pic mode:
    float zoom_value;
    float zoomoffset_x;
    float zoomoffset_y;

    // Preloaded textures:
    Texture *Folder16Tex;
    Texture *Folder64Tex;
    Texture *LoadingTex;

    // Used by the constructor to recursively populate the picture
    // collections from disk
    int Populate(fs::path path, int folder_idx);

    void ExitSinglePicView();

    void MoonlightShadow(float x, float y, int w, int h);
    void FoldersOnTheLeft();
    void PhotoStreamOnTheRight();
    void SingleChosenPic();

public:
    bool done;

    void KeyDown(int action);
    void RenderWindow();
    void LoadStuff();

    MiCasa(char *folderName);
    virtual ~MiCasa();
};

