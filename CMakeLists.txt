cmake_minimum_required(VERSION 3.25)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
project(tool-common VERSION 0.1.0 LANGUAGES CXX DESCRIPTION "Shared GUI libraries for LU-Rebuilt tools")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Guard: disable options when consumed as subdirectory
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(TOOL_COMMON_IS_TOP_LEVEL ON)
else()
    set(TOOL_COMMON_IS_TOP_LEVEL OFF)
endif()

# qt_common — standalone, no lu_assets dependency
find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)

    add_library(qt_common STATIC
        src/qt_common/file_browser.h
        src/qt_common/file_browser.cpp
    )
    target_link_libraries(qt_common PUBLIC Qt6::Widgets)
    target_compile_features(qt_common PUBLIC cxx_std_20)
    target_include_directories(qt_common PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/qt_common>
    )
    add_library(tool_common::qt_common ALIAS qt_common)
else()
    message(STATUS "Skipping qt_common (Qt6 not found)")
endif()

# gl_viewport — standalone OpenGL viewport, no lu_assets dependency
find_package(Qt6 COMPONENTS Widgets OpenGLWidgets QUIET)
find_package(OpenGL QUIET)

if(Qt6_FOUND AND OpenGL_FOUND)
    add_library(gl_viewport STATIC
        src/gl_viewport/render_mesh.h
        src/gl_viewport/orbit_camera.h src/gl_viewport/orbit_camera.cpp
        src/gl_viewport/gl_viewport_widget.h src/gl_viewport/gl_viewport_widget.cpp
        src/gl_viewport/ray_pick.h src/gl_viewport/ray_pick.cpp
        src/gl_viewport/gl_helpers.h src/gl_viewport/gl_helpers.cpp
        src/gl_viewport/transform_math.h src/gl_viewport/transform_math.cpp
    )
    target_link_libraries(gl_viewport PUBLIC
        Qt6::Widgets
        Qt6::OpenGLWidgets
        OpenGL::GL
    )
    target_compile_features(gl_viewport PUBLIC cxx_std_20)
    target_include_directories(gl_viewport PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/gl_viewport>
    )
    add_library(tool_common::gl_viewport ALIAS gl_viewport)
else()
    message(STATUS "Skipping gl_viewport (Qt6 or OpenGL not found)")
endif()
