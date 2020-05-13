/*
 * Mi Casa
 *
 * es Free Casa
 * 
 * TODOs:
 * 
 * - optimize skipping folders (when not visible) even more
 * - maybe far-away folders/pictures could be "squeezed"
 *     so the scrollbar doesn't get completely ridiculous
 * - right-button menu
 * - sorting (files & folders)
 * - jumping to folder from the left "needs some fixin'"
 * - video player needs polishing
 * - exif-data(?) / rotation(!)
 * - build system with autoconf/automake or cmake (?) or something
 *     or Ninja, if applicable, because it has the coolest name
 */

// Based on Dear imgui's standalone example application for SDL2 + OpenGL

#include <stdio.h>
#include <iostream>
#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include "vlclib-integration.h"
#include "texture.h"
#include "folders.h"
#include "vlc/vlc.h"

#include "resources/roboto-medium.c"
#include "resources/folder16x16.c"
#include "resources/folder64x64.c"
#include "resources/loading.c"

int main(int argc, char *argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    libvlc_instance_t *libvlc;
    libvlc_media_t *m;
    libvlc_media_player_t *mp = NULL;

    char const *vlc_argv[] = {
        "--no-xlib"	// Don't use Xlib.
    };
    int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
    int action = 0, pause = 0;

    struct context context;

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

    //SDL_Renderer *sdl_renderer = SDL_GetRenderer(window);
    context.renderer = SDL_CreateRenderer(window, -1, 0);

    context.texture = SDL_CreateTexture(
	context.renderer,
	SDL_PIXELFORMAT_BGR565, SDL_TEXTUREACCESS_STREAMING,
	VIDEOWIDTH, VIDEOHEIGHT);

    context.mutex = SDL_CreateMutex();

    // If you don't have this variable set you must have plugins directory
    // with the executable or libvlc_new() will not work!
    printf("VLC_PLUGIN_PATH=%s\n", getenv("VLC_PLUGIN_PATH"));

    // Initialise libVLC.
    libvlc = libvlc_new(vlc_argc, vlc_argv);
    if (libvlc == NULL) {
	std::cout << "LibVLC initialization failure." << std::endl;
	return EXIT_FAILURE;
    }


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); //(void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.IniFilename = NULL;

    io.Fonts->AddFontFromMemoryCompressedBase85TTF( roboto_medium_compressed_data_base85, 16.0f);

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    GLuint Folder16Tex = 0;
    LoadTextureFromMemory(folder16x16, sizeof(folder16x16), &Folder16Tex);
    GLuint Folder64Tex = 0;
    LoadTextureFromMemory(folder64x64, sizeof(folder64x64), &Folder64Tex);
    GLuint LoadingTex = 0;
    LoadTextureFromMemory(loading, sizeof(loading), &LoadingTex);

    Pic *loadAPic = NULL;
    Pic *currentPic = NULL;
    Folder *currentFolder = NULL;
    bool folderClicked = false;
    Folders *folders = new Folders(argc >= 1 ? argv[1] : NULL);
    bool frist = true;
    bool done = false;
    bool maybeDone = false;

    // Main loop
    while (!done)
    {
	action = 0;
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
                maybeDone = true;
	    } else if (event.type == SDL_KEYDOWN) {
		action = event.key.keysym.sym;
	    }
        }
        switch(action) {
            case SDLK_ESCAPE:
            case SDLK_q:
		if (mp != NULL) {
		    libvlc_media_player_stop(mp);
		    libvlc_media_player_release(mp);
		    mp = NULL;
		}
		if (currentPic != NULL) {
		    delete currentPic;
		    currentPic = NULL;
		} else {
		    maybeDone = true;
		}
                break;
            case ' ':
                printf("Pause toggle.\n");
                pause = !pause;
                break;
        }

	if (currentPic != NULL && currentPic->isVideo == true) {
	    if (mp == NULL) {
		m = libvlc_media_new_path(libvlc, currentPic->path->c_str());
		mp = libvlc_media_player_new_from_media(m);
		libvlc_media_release(m);

		libvlc_video_set_callbacks(mp, lock, unlock, display, &context);
		libvlc_video_set_format(mp, "RV16", VIDEOWIDTH, VIDEOHEIGHT, VIDEOWIDTH*2);
		libvlc_media_player_play(mp);
	    }
	    if(!pause) { context.n++; }
	    SDL_Delay(1000/10);

	} else {
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

	    ImGui::Columns(2, NULL);
	    if (frist) {
		ImGui::SetColumnWidth(0, io.DisplaySize.x / 5);
	    }
	    ImGui::BeginChild("ScrollRegion1", ImVec2(0, 0), false);
	    //
	    //	FOLDERS on the LEFT
	    //
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
	    ImGui::EndChild();

	    loadAPic = NULL;

	    ImGui::NextColumn();
	    //
	    //	HUGE EVERGROWING PICTURE REGION
	    //
	    //ImGui::Text("Content");
	    if (currentPic != NULL) {
		ImGui::BeginChild("ScrollRegion3", ImVec2(0, 0), false);
		//
		// WHEN A PICTURE HAS BEEN CHOSEN
		//
		float pw, ph;
		float cw = ImGui::GetColumnWidth();
		float ratio = (float) currentPic->width / currentPic->height;
		if (ratio >= 1.33f) {
		    pw = cw;
		    ph = cw / ratio;
		} else {
		    ph = cw * 3 / 4;
		    pw = ph * ratio;
		}
		ImVec2 cp = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(cp.x + (cw - pw) /2, cp.y + ((cw*3/4) - ph) / 2));
		ImGui::ImageButton((void*)(intptr_t)currentPic->texture, ImVec2(pw,ph), ImVec2(0.0f,0.0f), ImVec2(1.0f,1.0f), 1, ImVec4(0.80f, 0.80f, 0.83f, 1.00f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
		    delete currentPic;
		    currentPic = NULL;
		}

	    } else {
		ImGui::BeginChild("ScrollRegion2", ImVec2(0, 0), false);
		//
		// WHEN NO PICTURE HAS BEEN CHOSEN
		// (=PHOTO STREAM)
		//
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
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
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
	    ImGui::EndChild();
	    ImGui::End();
	    folderClicked = false;

	    if (maybeDone == true) {
		ImGui::Begin("Are you sure?", &maybeDone);
		ImGui::Text("Do you really want to leave Mi Casa?");
		if (ImGui::Button("Yes")) {
		    done = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("No")) {
		    maybeDone = false;
		}
		ImGui::End();
	    }

	    frist = false;
            ImGui::Render();
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            //glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
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
    }

    if (Folder16Tex != 0) {
	glDeleteTextures(1, &Folder16Tex);
    }
    if (Folder64Tex != 0) {
    	glDeleteTextures(1, &Folder64Tex);
    }
    if (LoadingTex != 0) {
    	glDeleteTextures(1, &LoadingTex);
    }

    // Stop stream and clean up libVLC.
    if (mp != NULL) {
	libvlc_media_player_stop(mp);
	libvlc_media_player_release(mp);
    }
    libvlc_release(libvlc);

    // Close window and clean up libSDL.
    SDL_DestroyMutex(context.mutex);
    SDL_DestroyRenderer(context.renderer);

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
