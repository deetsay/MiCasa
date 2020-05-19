/*
 * Mi Casa
 *
 * es Free Casa
 * 
 * TODOs por las tardes:
 * 
 * - back-button (& folder navigation!) when viewing a pic/video
 * - optimize skipping folders (when not visible) even more
 * - maybe far-away folders/pictures could be "squeezed"
 *     so the scrollbar doesn't get completely ridiculous
 *   - maybe the scrollbar should be a custom widget
 *   - "auto-closing" far-away folders?
 * - right-button menu
 * - sorting (files & folders)!!
 * - jumping to folder from the left "needs some fixin'"
 * - vlc integration uses deprecated methods, should probably look into that
 *   - and those string warnings in main.cpp
 * - exif-data(?) / rotation(!)
 */

// Based on Dear ImGui's standalone example application for SDL2 + OpenGL

#include <stdio.h>
#include <iostream>
#include <list>
#include <mutex>
#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include "vlc/vlc.h"
#include "texture.h"
#include "folder.h"
#include "vlclib-integration.h"

#include "resources/roboto-medium.c"
#include "resources/folder16x16.c"
#include "resources/folder64x64.c"
#include "resources/loading.c"

class MiCasa {
private:
    Texture *Folder16Tex;
    Texture *Folder64Tex;
    Texture *LoadingTex;

    VLCLibIntegration *vlc;
    Folder *rootFolder;
    Folder *miFolder;
    PicNode *miPicNode;
    Texture *miTexture;
    Pic *load_it;
    bool folderClicked;
    bool frist;

    int limit_w;
    int limit_h;

    float zoom_value;
    float zoomoffset_x;
    float zoomoffset_y;

    void ExitSinglePicView();

    void MoonlightShadow(float x, float y, int w, int h);
    void FoldersOnTheLeft(Folder *folder);
    void Photo(Folder *folder, Pic *pic);
    void PhotoStreamOnTheRight(Folder *folder);
    void SingleChosenPic();

public:
    bool done;

    void KeyDown(int action);
    void RenderWindow();
    void LoadStuff();

    MiCasa(char *folderName);
    virtual ~MiCasa();
};


MiCasa::MiCasa(char *folderName) {
    Folder16Tex = new Texture(folder16x16, sizeof(folder16x16));
    Folder64Tex = new Texture(folder64x64, sizeof(folder64x64));
    LoadingTex = new Texture(loading, sizeof(loading));

    done = false;
    load_it = NULL;
    miPicNode = NULL;
    miTexture = NULL;
    miFolder = NULL;
    folderClicked = false;
    frist = true;

    rootFolder = new Folder(folderName == NULL ? fs::current_path() : fs::path(folderName));

    vlc = new VLCLibIntegration();
}

void MiCasa::ExitSinglePicView() {
    vlc->bifurcate();
    if (miTexture != NULL) {
	delete miTexture;
	miTexture = NULL;
    }
    if (miPicNode != NULL) {
	PicNode *node = miPicNode->next;
	while (node != NULL) {
	    PicNode *next = node->next;
	    delete node;
	    node = next;
	}
	node = miPicNode->prev;
	while (node != NULL) {
	    PicNode *prev = node->prev;
	    delete node;
	    node = prev;
	}
	delete miPicNode;
	miPicNode = NULL;
    }
}

MiCasa::~MiCasa() {

    ExitSinglePicView();

    delete rootFolder;
    delete vlc;

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

void MiCasa::FoldersOnTheLeft(Folder *folder) {
    if (!folder->pictures.empty()) {
	//
	//	FOLDER WITH PICTURES -- clickable
	//
	ImGui::Image((void*)(intptr_t)Folder16Tex->texture, ImVec2(Folder16Tex->width, Folder16Tex->height));
	ImGui::SameLine();
	std::string s = folder->path->filename().string();
	const bool isSelected = (folder == miFolder);
	if (ImGui::Selectable(s.c_str(), isSelected)) {
	    folderClicked = true;
	    miFolder = folder;
	}
	//if (isSelected) ImGui::SetItemDefaultFocus();
    } else {
	//
	//	FOLDER WITHOUT PICTURES -- non-clickable
	//
	std::string s = folder->path->filename().string();
	ImGui::TextUnformatted(s.c_str());
	ImGui::SameLine();
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 p = ImGui::GetCursorScreenPos();
	draw_list->AddLine(ImVec2(p.x, p.y+8), ImVec2(p.x + ImGui::GetColumnWidth(), p.y+8),  ImGui::GetColorU32(ImGuiCol_Border));
	ImGui::SameLine();
	ImGui::TextUnformatted("");
    }
    for (Folder *child : folder->children) {
	FoldersOnTheLeft(child);
    }
}

void MiCasa::Photo(Folder *folder, Pic *pic) {
    float cw = ImGui::GetColumnWidth();
    ImVec2 cp = ImGui::GetCursorScreenPos();
    if (folderClicked == false && cp.y < ImGui::GetScrollY()+(ImGui::GetWindowSize().y/2)) {
	miFolder = folder;
    }

    int pw, ph;
    Texture *texture = pic->texture;
    if (texture == NULL) {
	if (pic->isVideo == false) {
	    load_it = pic;
	}
	texture = LoadingTex;
	texture->Fit(&pw, &ph, limit_w, limit_h);
    } else {
	if (texture->Fit(&pw, &ph, limit_w, limit_h) == false) {
	    if (pic->isVideo == false) {
		load_it = pic;
	    }
	}
    }

    int x = cp.x + (cw - pw) / 2;
    int y = cp.y + (cw - ph) / 2;
    ImGui::SetCursorScreenPos(ImVec2(x, y));
    ImGui::ImageButton((void*)(intptr_t)texture->texture, ImVec2(pw, ph),
	ImVec2(0.0f,0.0f), ImVec2(1.0f,1.0f), 0,
	ImVec4(0.80f, 0.80f, 0.83f, 1.00f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (ImGui::IsItemClicked()) {
	//miPic = pic;
	PicNode *prevNode = NULL;
	for (Pic *fldpic : folder->pictures) {
	    PicNode *newNode = new PicNode(fldpic);
	    newNode->prev = prevNode;
	    if (prevNode != NULL) {
		prevNode->next = newNode;
	    }
	    if (pic == fldpic) {
		miPicNode = newNode;
	    }
	    prevNode = newNode;
	}
    }
    MoonlightShadow(x, y, pw, ph);
    ImGui::NextColumn();
}

void MiCasa::PhotoStreamOnTheRight(Folder *folder) {
    // Instead of drawing items and then asking ImGui::IsItemVisible(),
    // use a miscellaneous combination of:
    // GetCursorPosY()			current position in scrolling area
    // GetScrollY()			visible window start
    // GetWindowSize().y?	visible window height    // SetCursorPos(ImVec2) -- to skip over lines without drawing anything
    if (ImGui::GetCursorPosY()+32 >= ImGui::GetScrollY()-100
	&& ImGui::GetCursorPosY() <= ImGui::GetScrollY()+ImGui::GetWindowSize().y+100) {

	ImGui::Columns(1);
	ImGui::Image((void *)(intptr_t)Folder64Tex->texture, ImVec2(Folder64Tex->width, Folder64Tex->height));
	ImGui::SameLine();
	std::string s = folder->path->string();
	ImGui::TextUnformatted(s.c_str());

	//if (folder->loaded == false) {
	//folder->load(limit_w, limit_h, LoadingTex, limit_w, limit_h);
	//}
	ImGui::NextColumn();

    } else {
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY()+32));
    }
    if (folderClicked == true && miFolder == folder) {
	//ImGui::SetItemDefaultFocus();
	//ImGui::SetScrollHereY();
	ImGui::SetScrollFromPosY(ImGui::GetCursorPosY()-ImGui::GetScrollY(), 0.5f);
    }

    bool haveColumnsBeenSet = false;
    std::forward_list<Pic*>::iterator pic_it = folder->pictures.begin();
    while (pic_it != folder->pictures.end()) {
	if (ImGui::GetCursorPosY()+limit_h >= ImGui::GetScrollY()-100
	    && ImGui::GetCursorPosY() <= ImGui::GetScrollY()+ImGui::GetWindowSize().y+100) {

	    if (haveColumnsBeenSet == false) {
		ImGui::Columns(5, NULL, false);
		haveColumnsBeenSet = true;
	    }
	    for (int i=0; i<5 && pic_it != folder->pictures.end(); i++, pic_it++) {
		Photo(folder, *pic_it);
	    }

	} else {
	    if (ImGui::GetCursorPosY()+limit_h < ImGui::GetScrollY()-20000
		|| ImGui::GetCursorPosY() > ImGui::GetScrollY()+ImGui::GetWindowSize().y+20000) {
		for (int i=0; i<5 && pic_it != folder->pictures.end(); i++, pic_it++) {
		    (*pic_it)->unload();
		}
	    } else if (ImGui::GetCursorPosY()+limit_h > ImGui::GetScrollY()-10000
		&& ImGui::GetCursorPosY() < ImGui::GetScrollY()+ImGui::GetWindowSize().y+10000) {
		for (int i=0; i<5 && pic_it != folder->pictures.end(); i++, pic_it++) {
		    Pic *pic = *pic_it;
		    if (load_it == NULL && pic->isVideo == false && pic->texture == NULL) {
			load_it = pic;
		    }
		}
	    } else {
		for (int i=0; i<5 && pic_it != folder->pictures.end(); i++, pic_it++) {
		}
	    }
	    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY()+limit_h));
	}
    }
    for (Folder *child : folder->children) {
	PhotoStreamOnTheRight(child);
    }
}

void MiCasa::SingleChosenPic() {
    ImGui::BeginChild("ScrollRegion3", ImVec2(0, 0), false);

    int pw, ph;
    float dw = ImGui::GetWindowContentRegionMax().x-4; //* 96 / 100;
    float dh = ImGui::GetWindowContentRegionMax().y-4; //* 96 / 100;

    if (miPicNode->pic->isVideo == true) {
	vlc->integrate(miTexture);
    }

    Texture *texture = miTexture;
    if (texture == NULL && miPicNode->pic->isVideo == false) {
	texture = miPicNode->pic->texture;
    }
    if (texture == NULL) {
	texture = LoadingTex;
    }

    float z = 64 / zoom_value;
    texture->Fit(&pw, &ph, (int) dw*z, (int) dh*z);

    ImVec2 cp = ImGui::GetCursorScreenPos();
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
    draw_list->AddImage((void*)(intptr_t)texture->texture, ImVec2(x, y), ImVec2(x+pw,y+ph), ImVec2(0.0f,0.0f), ImVec2(1.0f,1.0f), 0xffffffff);
    MoonlightShadow(x, y, pw, ph);

    //
    // OVERLAY
    //
    ImGui::SetCursorPos(cp);

    ImGui::Button("Back to folder");
    if (miPicNode != NULL && ImGui::IsItemClicked()) {
	ExitSinglePicView();
    }

    ImGui::EndChild();
}

void MiCasa::RenderWindow() {
    if (miPicNode != NULL) {
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
	FoldersOnTheLeft(rootFolder);
	ImGui::EndChild();

	load_it = NULL;

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
	PhotoStreamOnTheRight(rootFolder);
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
	    if (miPicNode != NULL) {
		ExitSinglePicView();
	    }
	    break;
	case SDLK_LEFT:
	    if (miPicNode != NULL && miPicNode->prev != NULL) {
		vlc->bifurcate();
		if (miTexture != NULL) {
		    delete miTexture;
		    miTexture = NULL;
		}
		miPicNode = miPicNode->prev;
	    }
	    break;
	case SDLK_RIGHT:
	    if (miPicNode != NULL && miPicNode->next != NULL) {
		vlc->bifurcate();
		if (miTexture != NULL) {
		    delete miTexture;
		    miTexture = NULL;
		}
		miPicNode = miPicNode->next;
	    }
	    break;
            //case ' ':
              //  printf("Pause toggle.\n");
              //  pause = !pause;
              //  break;
        }
}

void MiCasa::LoadStuff() {
    if (miPicNode != NULL && miTexture == NULL) {
	zoom_value = 64.0f;
	zoomoffset_x = 0.0f;
	zoomoffset_y = 0.0f;
	std::string s = miPicNode->pic->path->string();
	if (miPicNode->pic->isVideo == true) {
	    miTexture = vlc->integrationPreparation(s.c_str());
	} else {
	    miTexture = new Texture(s.c_str());
	}
    } else if (load_it != NULL) {
	if (load_it->isVideo == false) {
	    load_it->load(limit_w, limit_h);
	    load_it = NULL;
	}
    }
}

int main(int argc, char *argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Mi Casa", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 640, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); //(void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.IniFilename = NULL;

    io.Fonts->AddFontFromMemoryCompressedTTF(roboto_medium_compressed_data, roboto_medium_compressed_size, 16.0f);

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();
    //ImGuiStyle& style = ImGui::GetStyle();
    //style.FrameBorderSize = 0.0f;

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    MiCasa *m = new MiCasa(argc >= 1 ? argv[1] : NULL);

    // Main loop
    while (m->done == false)
    {
	int action = 0;
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                m->done = true;
	    } else if (event.type == SDL_KEYDOWN) {
		action = event.key.keysym.sym;
	    }
        }
	if (action == SDLK_f) {
	    if ((SDL_GetWindowFlags(window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0) {
		SDL_SetWindowFullscreen(window, 0);
	    } else {
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	    }
	}

	m->KeyDown(action);

	if (ImGui::IsMouseDoubleClicked(0)) {
	    if ((SDL_GetWindowFlags(window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0) {
		SDL_SetWindowFullscreen(window, 0);
	    } else {
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	    }
	}


	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

	ImGui::Begin("MiCasa Main", NULL,
	    ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoFocusOnAppearing
	    |ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
	    |ImGuiWindowFlags_NoMove);

	/*if (SDL_GetWindowFlags(window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP) == 0) {

	    if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Menu")) {
		    //ShowExampleMenuFile();
		    ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools")) {
		    ImGui::MenuItem("Metrics", NULL, &show_app_metrics);
		    ImGui::MenuItem("Style Editor", NULL, &show_app_style_editor);
		    ImGui::MenuItem("About Dear ImGui", NULL, &show_app_about);
		    ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	    }
	} else {
	}*/

	m->RenderWindow();

	ImGui::End();

	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

	//glClearColor(255, 255, 255, 255);
	//glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);

	m->LoadStuff();
    }
    
    delete m;

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
