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
#include <mutex>
#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include "vlc/vlc.h"
#include "texture.h"
#include "folders.h"
#include "vlclib-integration.h"

#include "resources/roboto-medium.c"
#include "resources/folder16x16.c"
#include "resources/folder64x64.c"
#include "resources/loading.c"

GLuint Folder16Tex = 0;
GLuint Folder64Tex = 0;
GLuint LoadingTex = 0;

VLCLibIntegration vlcinteg;

Folders *folders;
Folder *currentFolder;
Pic *currentPic;
Pic *loadAPic;
bool folderClicked;

void FoldersOnTheLeft() {
    ImGui::Text("Folders");
    Folder *folder = folders->first;
    while (folder != NULL) {
	if (folder->hasPictures == true) {
	    //
	    //	FOLDER WITH PICTURES -- clickable
	    //
	    ImGui::Image((void*)(intptr_t)Folder16Tex, ImVec2(16,16));
	    ImGui::SameLine();
	    std::string s = folder->path->filename().string();
	    const bool isSelected = folder == currentFolder;
	    if (ImGui::Selectable(s.c_str(), isSelected)) {
		if (currentPic != NULL) {
		    delete currentPic;
		    currentPic = NULL;
		}
		//if (currentFolder != NULL) {
		//    currentFolder->unload();
		//}
		folderClicked = true;
		currentFolder = folder;
	    }
	    //if (isSelected) ImGui::SetItemDefaultFocus();
	} else {
	    //
	    //	FOLDER WITHOUT PICTURES -- non-clickable
	    //
	    std::string s = folder->path->filename().string();
	    ImGui::Text(s.c_str());
	    ImGui::SameLine();
	    ImDrawList *draw_list = ImGui::GetWindowDrawList();
	    ImVec2 p = ImGui::GetCursorScreenPos();
	    draw_list->AddLine(ImVec2(p.x, p.y+8), ImVec2(p.x + ImGui::GetColumnWidth(), p.y+8),  ImGui::GetColorU32(ImGuiCol_Border));
	    ImGui::SameLine();
	    ImGui::Text("");
	}
	folder = folder->next;
    }
}

void PhotoStreamOnTheRight() {
    Folder *folder = folders->first;
    int limit_w = (int) ImGui::GetWindowContentRegionMax().x/6;
    int limit_h = limit_w*3/4;
    while (folder != NULL) {
	// Instead of drawing items and then asking ImGui::IsItemVisible(),
	// use a miscellaneous combination of:
	// GetCursorPosY()			current position in scrolling area
	// GetScrollY()			visible window start
	// GetWindowSize().y?	visible window height
	// SetCursorPos(ImVec2) -- to skip over lines without drawing anything
	if (ImGui::GetCursorPosY()+32 >= ImGui::GetScrollY()-100
	    && ImGui::GetCursorPosY() <= ImGui::GetScrollY()+ImGui::GetWindowSize().y+100) {

	    ImGui::Columns(1);
	    ImGui::Image((void *)(intptr_t)Folder64Tex, ImVec2(64,64));
	    ImGui::SameLine();
	    std::string s = folder->path->string();
	    ImGui::Text(s.c_str());

	    if (folder->loaded == false) {
		folder->load(limit_w, limit_h, LoadingTex, limit_w, limit_h);
	    }
	    ImGui::NextColumn();

	} else {
	    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY()+32));
	}
	if (folderClicked == true && currentFolder == folder) {
	    //ImGui::SetItemDefaultFocus();
	    //ImGui::SetScrollHereY();
	    ImGui::SetScrollFromPosY(ImGui::GetCursorPosY()-ImGui::GetScrollY(), 0.5f);
	}

	bool haveColumnsBeenSet = false;
	Pic *pic = folder->firstPic;
	while (pic != NULL) {
	    if (ImGui::GetCursorPosY()+limit_h >= ImGui::GetScrollY()-100
		&& ImGui::GetCursorPosY() <= ImGui::GetScrollY()+ImGui::GetWindowSize().y+100) {

		if (haveColumnsBeenSet == false) {
		    ImGui::Columns(5, NULL, false);
		    haveColumnsBeenSet = true;
		}

		float cw = ImGui::GetColumnWidth();
		for (int i=0; ((i<5) && (pic != NULL)); i++) {
		    if (pic->loaded == false) {
			loadAPic = pic;
		    }
		    ImVec2 cp = ImGui::GetCursorPos();
		    if (folderClicked == false && cp.y < ImGui::GetScrollY()+(ImGui::GetWindowSize().y/2)) {
			currentFolder = folder;
		    }
		    ImGui::SetCursorPos(ImVec2(cp.x + (cw - pic->width) /2, cp.y + (cw - pic->height) / 2));
		    ImGui::ImageButton((void*)(intptr_t)pic->texture, ImVec2(pic->width,pic->height), ImVec2(0.0f,0.0f), ImVec2(1.0f,1.0f), 0, ImVec4(0.80f, 0.80f, 0.83f, 1.00f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		    if (ImGui::IsItemClicked()) {
			if (currentPic != NULL) {
			    delete currentPic;
			}
			currentPic = new Pic(pic);

		    }
		    ImGui::NextColumn();
		    pic = pic->next;
		}

	    } else {
		if (ImGui::GetCursorPosY()+limit_h < ImGui::GetScrollY()-20000
		    || ImGui::GetCursorPosY() > ImGui::GetScrollY()+ImGui::GetWindowSize().y+20000) {
		    for (int i=0; ((i<5) && (pic != NULL)); i++) {
			if (pic->loaded == true) {
			    pic->unload();
			}
			pic = pic->next;
		    }
		} else if (ImGui::GetCursorPosY()+limit_h > ImGui::GetScrollY()-10000
		    && ImGui::GetCursorPosY() < ImGui::GetScrollY()+ImGui::GetWindowSize().y+10000) {
		    for (int i=0; i<5 && (pic != NULL); i++) {
			if (loadAPic == NULL && pic->loaded == false) {
			    loadAPic = pic;
			}
			pic = pic->next;
		    }
		} else {
		    for (int i=0; i<5 && (pic != NULL); i++) {
			pic = pic->next;
		    }
		}
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY()+limit_h));
	    }
	}
	folder = folder->next;
    }
}

void SingleChosenPic() {
    int pw, ph;
    int dw = ImGui::GetWindowContentRegionMax().x; //* 96 / 100;
    int dh = ImGui::GetWindowContentRegionMax().y; //* 96 / 100;

    if (currentPic->isVideo == true) {
	vlcinteg.integrate(currentPic);
    }

    Fit2U(&pw, &ph, currentPic->width, currentPic->height, dw, dh, true);
    ImVec2 cp = ImGui::GetCursorPos();


    ImGui::SetCursorPos(ImVec2(cp.x + (dw - pw) /2, cp.y + (dh - ph) / 2));

    //ImGui::ImageButton((void*)(intptr_t)currentPic->texture, ImVec2(pw,ph),
	//ImVec2(0.0f,0.0f), ImVec2(1.0f,1.0f), 1, ImVec4(0.80f, 0.80f, 0.83f, 1.00f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (currentPic->reallyLoaded == false) {
	ImGui::Image((void*)(intptr_t)currentPic->texture, ImVec2(pw,ph),
	    //ImVec2((float)4/currentPic->width,(float)4/currentPic->height),
	    ImVec2(0.0f,0.0f),
	    //ImVec2(0.7f,0.7f),
	    ImVec2(
		(float)(currentPic->width-4)/currentPic->width,
		(float)(currentPic->height-4)/currentPic->height),
	    ImVec4(1.0f, 1.8f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    } else {
	ImGui::Image((void*)(intptr_t)currentPic->texture, ImVec2(pw,ph),
	    ImVec2(0.0f,0.0f), ImVec2(1.0f,1.0f), ImVec4(1.0f, 1.8f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
	vlcinteg.bifurcate();
	delete currentPic;
	currentPic = NULL;
    }

    ImGui::SetCursorPos(cp);

    ImGui::Button("Back to folder");
    if (currentPic != NULL && ImGui::IsItemClicked()) {
	vlcinteg.bifurcate();
	delete currentPic;
	currentPic = NULL;
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

    io.Fonts->AddFontFromMemoryCompressedBase85TTF( roboto_medium_compressed_data_base85, 16.0f);

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    LoadTextureFromMemory(folder16x16, sizeof(folder16x16), &Folder16Tex);
    LoadTextureFromMemory(folder64x64, sizeof(folder64x64), &Folder64Tex);
    LoadTextureFromMemory(loading, sizeof(loading), &LoadingTex);

    loadAPic = NULL;
    currentPic = NULL;
    currentFolder = NULL;
    folderClicked = false;
    bool frist = true;
    bool done = false;

    folders = new Folders(argc >= 1 ? argv[1] : NULL);

    // Main loop
    while (!done)
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
                done = true;
	    } else if (event.type == SDL_KEYDOWN) {
		action = event.key.keysym.sym;
	    }
        }
        switch(action) {
            case SDLK_q:
		done = true;
		break;
            case SDLK_ESCAPE:
		if (currentPic != NULL) {
		    vlcinteg.bifurcate();
		    delete currentPic;
		    currentPic = NULL;
		}
                break;
	    case SDLK_f:
		if (SDL_GetWindowFlags(window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
		    SDL_SetWindowFullscreen(window, 0);
		} else {
		    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
		break;
	    case SDLK_LEFT:
		if (currentPic != NULL && currentPic->prev != NULL) {
		    vlcinteg.bifurcate();
		    Pic *pic = new Pic(currentPic->prev);
		    delete currentPic;
		    currentPic = pic;
		}
		break;
	    case SDLK_RIGHT:
		if (currentPic != NULL && currentPic->next != NULL) {
		    vlcinteg.bifurcate();
		    Pic *pic = new Pic(currentPic->next);
		    delete currentPic;
		    currentPic = pic;
		}
		break;
            //case ' ':
              //  printf("Pause toggle.\n");
              //  pause = !pause;
              //  break;
        }

	{
	    ImGui_ImplOpenGL2_NewFrame();
	    ImGui_ImplSDL2_NewFrame(window);
	    ImGui::NewFrame();

	    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	    ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

	    ImGui::Begin("MiCasa Main", NULL,
    		ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoFocusOnAppearing
		|ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
		|ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);

	    if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Menu")) {
		    //ShowExampleMenuFile();
		    ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools")) {
		    /*ImGui::MenuItem("Metrics", NULL, &show_app_metrics);
		    ImGui::MenuItem("Style Editor", NULL, &show_app_style_editor);
		    ImGui::MenuItem("About Dear ImGui", NULL, &show_app_about);*/
		    ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	    }

	    if (currentPic != NULL) {
		ImGui::BeginChild("ScrollRegion3", ImVec2(0, 0), false);
		//
		// WHEN A PICTURE HAS BEEN CHOSEN
		//
		SingleChosenPic();
		ImGui::EndChild();

	    } else {

		ImGui::Columns(2, NULL);
		if (frist) {
		    ImGui::SetColumnWidth(0, io.DisplaySize.x / 5);
		}
		ImGui::BeginChild("ScrollRegion1", ImVec2(0, 0), false);
		//
		//	FOLDERS on the LEFT
		//
		FoldersOnTheLeft();
		ImGui::EndChild();

		loadAPic = NULL;

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
		PhotoStreamOnTheRight();
		ImGui::EndChild();
	    }
	    ImGui::End();
	    folderClicked = false;

	    frist = false;
            ImGui::Render();
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

            //glClearColor(255, 255, 255, 255);
            //glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(window);

	    if (currentPic != NULL && currentPic->loaded == false) {
		currentPic->load();
	    } else if (loadAPic != NULL) {
		loadAPic->load();
		loadAPic = NULL;
	    }
        }
    }

    if (currentPic != NULL) {
	delete currentPic;
	currentPic = NULL;
    }

    delete folders;

    if (Folder16Tex != 0) {
	glDeleteTextures(1, &Folder16Tex);
    }
    if (Folder64Tex != 0) {
    	glDeleteTextures(1, &Folder64Tex);
    }
    if (LoadingTex != 0) {
    	glDeleteTextures(1, &LoadingTex);
    }

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
