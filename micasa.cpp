#include "micasa.h"

#include <iostream>
#include <set>
#include <regex>

#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"

#include "resources/folder16x16.c"
#include "resources/folder64x64.c"
#include "resources/loading.c"

namespace fs = std::filesystem;

const std::regex pic_regex("^[^\\.].*\\.(jpg|jpeg|gif|png|mpg|mp4|avi|3gp)$", std::regex_constants::icase);

int MiCasa::Populate(fs::path path, int folder_idx) {

    //std::cout << "Populating folder " << path << " idx " << folder_idx << std::endl;

    fs::directory_iterator it = fs::directory_iterator(path);

    std::set<std::string> pop_pics;
    std::set<fs::path> pop_folders;
    int running_idx = folder_idx;

    for (const fs::directory_entry &entry : it) {
	if (fs::is_directory(entry)) {
	    pop_folders.insert(entry.path());

    	} else if (fs::is_regular_file(entry)) {
	    std::smatch pic_match;
	    std::string picname = entry.path().filename().string();
	    if (std::regex_match(picname, pic_match, pic_regex)) {
		pop_pics.insert(entry.path());
	    }
	}
    }
    // Adding is done only *after* the above loop on purpose,
    // because now the pics are sorted (by std::set)!
    for (std::string filename : pop_pics) {
	photostream.emplace_back(filename, 0);
	running_idx++;
    }
    if (running_idx > folder_idx || !pop_folders.empty()) {
	folders.emplace_back(path, folder_idx);
	for (fs::path p : pop_folders) {
	    running_idx = Populate(p, running_idx);
	}
	if (running_idx == folder_idx) {
	    folders.pop_back();
	}
    }
    return running_idx;
}

MiCasa::MiCasa(char *folderName) {
    Folder16Tex = new Texture(folder16x16, sizeof(folder16x16));
    Folder64Tex = new Texture(folder64x64, sizeof(folder64x64));
    LoadingTex = new Texture(loading, sizeof(loading));

    done = false;
    texture = NULL;
    chosen_folder = -1;
    folderClicked = false;
    frist = true;
    chosen_pic = -1;

    std::cout << "Populating picture folders." << std::endl;

    Populate(folderName == NULL ? fs::current_path() : fs::path(folderName), 0);
    folders.shrink_to_fit();
    photostream.shrink_to_fit();

    std::cout << "Photostream size: " << photostream.size() << std::endl;

    std::cout << "Initializing background loader." << std::endl;

    loader = new Loader(&photostream, &previews);

    std::cout << "Integrating VLCLib." << std::endl;

    vlc = new VLCLibIntegration();
}

void MiCasa::ExitSinglePicView() {
    vlc->bifurcate();
    if (texture != NULL) {
	delete texture;
	texture = NULL;
    }
    chosen_pic = -1;
}

MiCasa::~MiCasa() {

    ExitSinglePicView();

    delete loader;

    delete vlc;

    for (std::pair<const int, Texture*> pt : previews) {
	delete pt.second;
    }

    delete Folder16Tex;
    delete Folder64Tex;
    delete LoadingTex;

}


void MiCasa::MoonlightShadow(float x, float y, int w, int h) {
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->AddLine(ImVec2(x+w, y), ImVec2(x+w, y+h),  0xff666666, 1.0f);
    draw_list->AddLine(ImVec2(x, y+h), ImVec2(x+w+1, y+h),  0xff666666, 1.0f);
    x++; y++;
    draw_list->AddLine(ImVec2(x+w, y), ImVec2(x+w, y+h),  0xff888888, 1.0f);
    draw_list->AddLine(ImVec2(x, y+h), ImVec2(x+w+1, y+h),  0xff888888, 1.0f);
    x++; y++;
    draw_list->AddLine(ImVec2(x+w, y), ImVec2(x+w, y+h),  0xffaaaaaa, 1.0f);
    draw_list->AddLine(ImVec2(x, y+h), ImVec2(x+w+1, y+h),  0xffaaaaaa, 1.0f);
    x++; y++;
    draw_list->AddLine(ImVec2(x+w, y), ImVec2(x+w, y+h),  0xffcccccc, 1.0f);
    draw_list->AddLine(ImVec2(x, y+h), ImVec2(x+w+1, y+h),  0xffcccccc, 1.0f);
}

void MiCasa::FoldersOnTheLeft() {
    for (int fi = 0; fi < folders.size(); fi++) {
	int ni;
	if ((fi + 1) < folders.size()) {
	    ni = folders[fi+1].pic;
	} else {
	    ni = photostream.size();
	}
	if (folders[fi].pic == ni) {
	    //
	    //	FOLDER WITHOUT PICTURES -- non-clickable
	    //
	    std::string s = folders[fi].path.filename().string();
	    ImGui::TextUnformatted(s.c_str());
	    ImGui::SameLine();
	    ImDrawList *draw_list = ImGui::GetWindowDrawList();
	    ImVec2 p = ImGui::GetCursorScreenPos();
	    draw_list->AddLine(ImVec2(p.x, p.y+8), ImVec2(p.x + ImGui::GetColumnWidth(), p.y+8),  ImGui::GetColorU32(ImGuiCol_Border));
	    ImGui::SameLine();
	    ImGui::TextUnformatted("");
	} else {
	    //
	    //	FOLDER WITH PICTURES -- clickable
	    //
	    int w, h;
	    void *tex = Folder16Tex->Fit(&w, &h);
	    ImGui::Image(tex, ImVec2(w, h));
	    ImGui::SameLine();
	    std::string s = folders[fi].path.filename().string();
	    const bool isSelected = (chosen_folder == fi);
	    if (ImGui::Selectable(s.c_str(), isSelected)) {
		folderClicked = true;
		chosen_folder = fi;
	    }
	}
    }
}

// Instead of drawing items and then asking ImGui::IsItemVisible(),
// use a miscellaneous combination of:
// GetCursorPosY()			current position in scrolling area
// GetScrollY()			visible window start
// GetWindowSize().y?	visible window height    // SetCursorPos(ImVec2) -- to skip over lines without drawing anything
void MiCasa::PhotoStreamOnTheRight() {
    bool manyColumns = false;
    int fvp=-1;
    int lvp=-1;
    int fi=0;
    int i=0;
    while (fi < folders.size()) {
	int li;
	if (fi+1 < folders.size()) {
	    li = folders.at(fi+1).pic;
	} else {
	    li = photostream.size();
	}

	// Visible folder
	if (ImGui::GetCursorPosY()+32 >= ImGui::GetScrollY()-100
	    && ImGui::GetCursorPosY() <= ImGui::GetScrollY()+ImGui::GetWindowSize().y+100) {

	    ImGui::Columns(1);
	    manyColumns = false;
	    int w, h;
	    void *tex = Folder64Tex->Fit(&w, &h);
	    ImGui::Image(tex, ImVec2(w, h));
	    ImGui::SameLine();
	    std::string s = folders.at(fi).path.string();
	    ImGui::TextUnformatted(s.c_str());

	    //if (folder->loaded == false) {
	    //folder->load(limit_w, limit_h, LoadingTex, limit_w, limit_h);
	    //}
	    ImGui::NextColumn();

	// Invisible folder
	} else {
	    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY()+32));
	}

	// Folder clicked from the left
	if (folderClicked == true && fi == chosen_folder) {
	    //ImGui::SetItemDefaultFocus();
	    //ImGui::SetScrollHereY();
	    ImGui::SetScrollFromPosY(ImGui::GetCursorPosY()-ImGui::GetScrollY(), 0.5f);
	}

	while (i < li) {

	    if (ImGui::GetCursorPosY()+limit_h >= ImGui::GetScrollY()-100
		&& ImGui::GetCursorPosY() <= ImGui::GetScrollY()+ImGui::GetWindowSize().y+100) {

		if (manyColumns == false) {
		    ImGui::Columns(5, NULL, false);
		    manyColumns = true;
		}
		if (fvp<0) { fvp=i; }
		lvp = i;

		for (int cx=0; cx<5 && i<li; cx++, i++) {
		    float cw = ImGui::GetColumnWidth();
		    ImVec2 cp = ImGui::GetCursorScreenPos();
		    if (folderClicked == false && cp.y < ImGui::GetScrollY()+(ImGui::GetWindowSize().y/2)) {
			chosen_folder = fi;
		    }

		    int pw, ph;
		    Texture *tex;
		    std::unordered_map<int, Texture*>::iterator pt = previews.find(i);
		    if (pt == previews.end()) {
			tex = LoadingTex;
		    } else {
			tex = pt->second;
		    }
		    void *gltex = tex->Fit(&pw, &ph, limit_w, limit_h);

		    int x = cp.x + (cw - pw) / 2;
		    int y = cp.y + (cw - ph) / 2;
		    ImGui::SetCursorScreenPos(ImVec2(x, y));
		    ImGui::ImageButton(gltex, ImVec2(pw, ph),
			ImVec2(0.0f,0.0f), ImVec2(1.0f,1.0f), 0,
			ImVec4(0.80f, 0.80f, 0.83f, 1.00f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		    if (ImGui::IsItemClicked()) {
			chosen_pic = i;
		    }
		    MoonlightShadow(x, y, pw, ph);
		    ImGui::NextColumn();
		}

	    } else {
		for (int cx=0; cx<5 && i<li; cx++, i++) {
		}
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY()+limit_h));
	    }
	}
	fi++;
    }
    loader->fyi(fvp, lvp, limit_w, limit_h);
}

void MiCasa::SingleChosenPic() {
    ImGui::BeginChild("ScrollRegion3", ImVec2(0, 0), false);

    int pw, ph;
    float dw = ImGui::GetWindowSize().x; //* 96 / 100;
    float dh = ImGui::GetWindowSize().y; //* 96 / 100;

    Pic pic = photostream[chosen_pic];
    if (pic.is_video()) {
	vlc->integrate(texture);
    }

    Texture *t = texture;
    if (t == NULL && !pic.is_video()) {
	std::unordered_map<int, Texture*>::iterator pt = previews.find(chosen_pic);
	if (pt != previews.end()) {
	    t = pt->second;
	}
    }
    if (t == NULL) {
	t = LoadingTex;
    }

    float z = 64 / zoom_value;
    void *gltex = t->Fit(&pw, &ph, (int) dw*z, (int) dh*z);

    ImVec2 cp = ImVec2(0.0f, 0.0f);//ImGui::GetCursorScreenPos();
    float mid_x = cp.x + (dw / 2);	// middle point of the "picture area"
    float mid_y = cp.y + (dh / 2);

    ImGuiIO& io = ImGui::GetIO();
    float wheel = io.MouseWheel;

    // While the wheel isn't being moved, don't move the picture
    if (wheel != 0.0f) {
	//
	// ZOOM OFFSET CALCULATION
	//
	// enforce zoom limits
	float zoom_temp = zoom_value;
	zoom_value -= wheel;
	if (zoom_value < 1.0f) {
	    zoom_value = 1.0f;
	} else if (zoom_value > 64.0f) {
	    zoom_value = 64.0f;
	}
	zoom_temp -= zoom_value;
	
	if (zoom_temp < 0.0f) {

	    zoomoffset_x += (zoomoffset_x * zoom_temp) / 16;
	    zoomoffset_y += (zoomoffset_y * zoom_temp) / 16;

	} else if (zoom_temp > 0.0f) {

	    // difference from center to mouse
	    float mx = mid_x - io.MousePos.x;
	    float my = mid_y - io.MousePos.y;
	    zoomoffset_x += mx * zoom_temp * z / 16;
	    zoomoffset_y += my * zoom_temp * z / 16;
	}
    }

    float x = mid_x - (pw / 2) + zoomoffset_x;
    float y = mid_y - (ph / 2) + zoomoffset_y;

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->AddImage(gltex, ImVec2(x, y), ImVec2(x+pw,y+ph), ImVec2(0.0f,0.0f), ImVec2(1.0f,1.0f), 0xffffffff);
    MoonlightShadow(x, y, pw, ph);

    //
    // OVERLAY
    //
    ImGui::SetCursorPos(cp);

    ImGui::Button("Back to folder");
    if (chosen_pic >= 0 && ImGui::IsItemClicked()) {
	ExitSinglePicView();
    }

    ImGui::EndChild();
}

void MiCasa::RenderWindow() {
    if (chosen_pic >= 0) {
	//
	// WHEN A PICTURE HAS BEEN CHOSEN
	//
	SingleChosenPic();

    } else {

	ImGui::Columns(2, NULL);
	if (frist) {
	    ImGui::SetColumnWidth(0, ImGui::GetIO().DisplaySize.x / 5);
	}
	ImGui::BeginChild("ScrollRegion1", ImVec2(0, 0), false);
	//
	//	FOLDERS on the LEFT
	//
	ImGui::TextUnformatted("Folders");
	FoldersOnTheLeft();
	ImGui::EndChild();

	ImGui::NextColumn();
	//
	//	HUGE EVERGROWING PICTURE REGION
	//
	//ImGui::Text("Content");

	ImGui::BeginChild("ScrollRegion2", ImVec2(0, 0), false);
	//
	// WHEN NO PICTURE HAS BEEN CHOSEN
	// (=PHOTO STREAM)
	//
	limit_w = (int) ImGui::GetWindowContentRegionMax().x/6;
	limit_h = limit_w*3/4;
	PhotoStreamOnTheRight();
	ImGui::EndChild();
    }
    folderClicked = false;
    frist = false;
}

void MiCasa::KeyDown(int action) {
    switch(action) {
	case SDLK_q:
	    done = true;
	    break;
	case SDLK_ESCAPE:
	    if (chosen_pic >= 0) {
		ExitSinglePicView();
	    }
	    break;
	case SDLK_LEFT:
	    if (chosen_pic > 0) {
		vlc->bifurcate();
		if (texture != NULL) {
		    delete texture;
		    texture = NULL;
		}
		chosen_pic--;
	    }
	    break;
	case SDLK_RIGHT:
	    if (chosen_pic < photostream.size()-1) {
		vlc->bifurcate();
		if (texture != NULL) {
		    delete texture;
		    texture = NULL;
		}
		chosen_pic++;
	    }
	    break;
            //case ' ':
              //  printf("Pause toggle.\n");
              //  pause = !pause;
              //  break;
        }
}

void MiCasa::LoadStuff() {
    if (chosen_pic >= 0 && texture == NULL) {
	zoom_value = 64.0f;
	zoomoffset_x = 0.0f;
	zoomoffset_y = 0.0f;
	Pic pic = photostream[chosen_pic];
	std::string s = pic.path;
	if (pic.is_video()) {
	    texture = vlc->integrationPreparation(s.c_str());
	} else {
	    texture = new Texture(s.c_str());
	}
    }
}

