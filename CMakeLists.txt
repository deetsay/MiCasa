###############################################################################

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

#include(FindPkgConfig)
include(CPack)
include(InstallRequiredSystemLibraries)

################################################################ Project config

project(micasa VERSION 0.1)

configure_file(micasa-config.h.in micasa-config.h)

list(APPEND SOURCES main.cpp folders.cpp texture.cpp)
list(APPEND SOURCES vlclib-integration.cpp)

################ RPATH stuff for OSX, hope it doesn't interfere with other OSes
# MAY have been unnecessary after all
## use, i.e. don't skip the full RPATH for the build tree
#set(CMAKE_SKIP_BUILD_RPATH FALSE)
#
## when building, don't use the install RPATH already
## (but later on when installing)
#set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
#
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#
## add the automatically determined parts of the RPATH
## which point to directories outside the build tree to the install RPATH
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#
## the RPATH to be used when installing, but only if it's not a system directory
#list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
#if("${isSystemDir}" STREQUAL "-1")
#    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#endif("${isSystemDir}" STREQUAL "-1")

############################################################## Everything ImGui

set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/imgui)
list(APPEND INCLUDES ${IMGUI_DIR})
list(APPEND SOURCES ${IMGUI_DIR}/imgui.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_draw.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_impl_opengl2.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_impl_sdl.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_widgets.cpp)

############################################################### Everything SDL2

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
list(APPEND INCLUDES ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
list(APPEND LIBS ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})

############################################################# Everything OpenGL

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
list(APPEND INCLUDES ${OPENGL_INCLUDE_DIRS})
list(APPEND LIBS ${OPENGL_LIBRARIES})

################################################################ Everything VLC

find_package(LIBVLC REQUIRED)
list(APPEND INCLUDES ${LIBVLC_INCLUDE_DIR})
list(APPEND LIBS ${LIBVLC_LIBRARY})

################################################################## Build config

add_executable(micasa ${SOURCES})
target_link_libraries(micasa PUBLIC ${LIBS})
target_include_directories(micasa PUBLIC ${PROJECT_BINARY_DIR} ${INCLUDES})

if (CMAKE_BUILD_TYPE STREQUAL Release)
  add_custom_command(
    TARGET micasa POST_BUILD
    COMMAND ${CMAKE_STRIP} micasa
  )
endif ()

################################################################ Install config

install(TARGETS micasa
	RUNTIME DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/micasa-config.h" DESTINATION include)

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${micasa_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${micasa_VERSION_MINOR}")

###############################################################################
