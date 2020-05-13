#######################################################################

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

include(FindPkgConfig)
include(CPack)
include(InstallRequiredSystemLibraries)

######################################################## Project config

project(micasa VERSION 0.1)

configure_file(micasa-config.h.in micasa-config.h)

list(APPEND SOURCES main.cpp folders.cpp texture.cpp)
list(APPEND SOURCES vlclib-integration.cpp)

###################################################### Everything ImGui

set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/imgui)
list(APPEND INCLUDES ${IMGUI_DIR})
list(APPEND SOURCES ${IMGUI_DIR}/imgui.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_draw.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_impl_opengl2.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_impl_sdl.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_widgets.cpp)

####################################################### Everything SDL2

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image)
list(APPEND INCLUDES ${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})
list(APPEND LIBS ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES})

##################################################### Everything OpenGL

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
list(APPEND INCLUDES ${OPENGL_INCLUDE_DIRS})
list(APPEND LIBS ${OPENGL_LIBRARIES})

######################################################## Everything VLC

pkg_search_module(LIBVLC REQUIRED libvlc)
list(APPEND INCLUDES ${LIBVLC_INCLUDE_DIRS})
list(APPEND LIBS ${LIBVLC_LIBRARIES})

########################################################## Build config

add_executable(micasa ${SOURCES})
target_link_libraries(micasa PUBLIC ${LIBS})
target_include_directories(micasa PUBLIC ${PROJECT_BINARY_DIR} ${INCLUDES})

if (CMAKE_BUILD_TYPE STREQUAL Release)
  add_custom_command(
    TARGET micasa POST_BUILD
    COMMAND ${CMAKE_STRIP} micasa
  )
endif ()

######################################################## Install config

install(TARGETS micasa DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/micasa-config.h" DESTINATION include)

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${micasa_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${micasa_VERSION_MINOR}")

#######################################################################
