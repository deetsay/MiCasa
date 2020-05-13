#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need SDL2 (http://www.libsdl.org):
# Linux:
#   apt-get install libsdl2-dev
# Mac OS X:
#   brew install sdl2
# MSYS2:
#   pacman -S mingw-w64-i686-SDL
#

#CXX = g++
#CXX = clang++

SOURCES = imgui_impl_sdl.cpp imgui_impl_opengl2.cpp
SOURCES += imgui.cpp imgui_draw.cpp imgui_widgets.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)

CXXFLAGS = -I../ -I../../
CXXFLAGS += -g -Wall -Wformat
LIBS =

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	CXXFLAGS += `sdl2-config --cflags`
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	CXXFLAGS += `sdl2-config --cflags`
	CXXFLAGS += -I/usr/local/include -I/opt/local/include
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
	CXXFLAGS += `pkg-config --cflags sdl2`
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:	%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all:	$(OBJS)

clean:
	rm -f $(OBJS)
