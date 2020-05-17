###############################################################################

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to 'Release' as none was specified.")
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

include(FindPkgConfig)
include(InstallRequiredSystemLibraries)

################################################################ Project config

project(micasa VERSION 0.1)

configure_file(micasa-config.h.in micasa-config.h)

list(APPEND SOURCES main.cpp folder.cpp texture.cpp)
list(APPEND SOURCES vlclib-integration.cpp)

############################################################## Everything ImGui

set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/imgui)
list(APPEND INCLUDES ${IMGUI_DIR})
list(APPEND SOURCES ${IMGUI_DIR}/imgui.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_draw.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_impl_opengl2.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_impl_sdl.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_widgets.cpp)

############################################################### Everything SDL2

if(PKG_CONFIG_FOUND)
	pkg_search_module(SLD2 REQUIRED sdl2)
	pkg_search_module(SLD2IMAGE REQUIRED SDL2_image)
	list(APPEND INCLUDES ${SLD2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})
	list(APPEND LIBS ${SDL2_LIBRARIES} ${SLD2IMAGE_LIBRARIES})
else()
	find_package(SDL2 REQUIRED)
	find_package(SDL2_image REQUIRED)
	list(APPEND INCLUDES ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
	list(APPEND LIBS ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
endif()

############################################################# Everything OpenGL

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
list(APPEND INCLUDES ${OPENGL_INCLUDE_DIRS})
list(APPEND LIBS ${OPENGL_LIBRARIES})

################################################################ Everything VLC

if(PKG_CONFIG_FOUND AND NOT APPLE)
	pkg_search_module(LIBVLC REQUIRED libvlc)
	list(APPEND INCLUDES ${LIBVLC_INCLUDE_DIRS})
	list(APPEND LIBS ${LIBVLC_LIBRARIES})
else()
	find_package(LIBVLC REQUIRED)
	list(APPEND INCLUDES ${LIBVLC_INCLUDE_DIR})
	list(APPEND LIBS ${LIBVLC_LIBRARY})
endif()

################################################################## Build config

add_executable(micasa ${SOURCES})
target_link_libraries(micasa PUBLIC ${LIBS})
target_include_directories(micasa PUBLIC ${PROJECT_BINARY_DIR} ${INCLUDES})

if(CMAKE_BUILD_TYPE STREQUAL Release)
	add_custom_command(
		TARGET micasa POST_BUILD
		COMMAND ${CMAKE_STRIP} micasa
	)
endif()

################################################################ Install config

install(TARGETS micasa
	RUNTIME DESTINATION bin)
#install(FILES "${PROJECT_BINARY_DIR}/micasa-config.h" DESTINATION include)

set(CPACK_GEERATOR "TGZ;DEB")
set(CPACK_PACKAGE_NAME "micasa")
set(CPACK_PACKAGE_VENDOR "Tero Mäyränen")
set(CPACK_PACKAGE_CONTACT "tero@tero.fi")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Mi Casa is a home media browser")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "")
#set(CPACK_RESOURCE_FILE_WELCOME "")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${micasa_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${micasa_VERSION_MINOR}")
set(CPACK_SOURCE_GENERATOR "STGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}_Source.tar.gz")
set(CPACK_SOURCE_IGNORE_FILES "/build/;.git*")
set(CPACK_SOURCE_PACKAGE_FILE_NAME
	"${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
set(CPACK_VERBATIM_VARIABLES true)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS true)
include (CPack)

###############################################################################
