/*
 * Mi Casa
 *
 * es Free Casa
 * 
 * TODOs por las tardes:
 * 
 * - video thumbnails!
 * - rotation! EXIF-data? custom property file?
 * - transparent folder navigation window
 * - video controls
 * - slideshows!
 * - sorting (..the folders?) in different ways?
 *
 * (all the rest are related to the folder UI):
 * - optimize skipping folders (when not visible) even more
 * - maybe far-away folders/pictures could be "squeezed"
 *     so the scrollbar doesn't get completely ridiculous
 *   - maybe the scrollbar should be a custom widget
 *     - scrolling is sometimes jumpy now anyway
 * - jumping to a folder from the left is not entirely reliable...
 */

// Based on Dear ImGui's standalone example application for SDL2 + OpenGL

#include "micasa.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"

#include "resources/roboto-medium.c"

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
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameBorderSize = 0.0f;
    style.FramePadding = ImVec2(0.0f,0.0f);
    style.WindowBorderSize = 0.0f;
    style.WindowPadding = ImVec2(0.0f,0.0f);

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    //style.GrabRounding = 0.0f;
    //style.PopupRounding = 0.0f;
    //style.ScrollbarRounding = 0.0f;

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
