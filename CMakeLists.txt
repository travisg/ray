cmake_minimum_required(VERSION 3.20)
project(ray LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# RelWithDebInfo (-O2 -g) by default, with its -DNDEBUG removed so asserts stay on.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo)
endif()
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

option(RAY_NATIVE_ARCH "Compile with -march=native" OFF)
option(RAY_LTO "Link-time optimization" OFF)
option(RAY_PROFILE "Compile and link with -pg" OFF)
option(RAY_WERROR "Treat warnings as errors" OFF)
option(RAY_BUILD_TESTS "Build the test programs" ON)

find_package(Threads REQUIRED)

# Flags shared by every target.
add_library(ray_flags INTERFACE)
target_compile_options(ray_flags INTERFACE -Wall -Wextra)
if(RAY_WERROR)
    target_compile_options(ray_flags INTERFACE -Werror)
endif()
if(RAY_NATIVE_ARCH)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-march=native RAY_HAVE_MARCH_NATIVE)
    if(RAY_HAVE_MARCH_NATIVE)
        target_compile_options(ray_flags INTERFACE -march=native)
    else()
        message(WARNING "compiler does not accept -march=native; RAY_NATIVE_ARCH ignored")
    endif()
endif()
if(RAY_PROFILE)
    target_compile_options(ray_flags INTERFACE -pg)
    target_link_options(ray_flags INTERFACE -pg)
endif()
if(RAY_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT RAY_IPO_OK OUTPUT RAY_IPO_MSG LANGUAGES CXX)
    if(RAY_IPO_OK)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else()
        message(WARNING "LTO unavailable: ${RAY_IPO_MSG}")
    endif()
endif()

# .ray stream reader/writer, shared by the renderer and the tools.
add_library(rayfile STATIC ray/ray_file.cpp)
target_include_directories(rayfile PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(rayfile PUBLIC ray_flags)

# Renderer core: scene, tracing, work distribution, drawables, shaders, meshes.
add_library(raycore STATIC
    ray/scene.cpp
    ray/scene_library.cpp
    ray/tracer.cpp
    ray/trace_master.cpp
    ray/render_surface.cpp
    ray/framebuffer.cpp
    ray/pfm.cpp
    ray/tev_display.cpp
    ray/drawables/mesh_drawable.cpp
    ray/drawables/plane.cpp
    ray/drawables/sphere.cpp
    ray/drawables/triangle.cpp
    ray/shaders/debug_shader.cpp
    ray/shaders/default_shader.cpp
    ray/mesh/geometry.cpp
    ray/mesh/mesh.cpp
    ray/mesh/obj_loader.cpp
    ray/mesh/surface.cpp
)
target_link_libraries(raycore PUBLIC rayfile Threads::Threads)

add_executable(ray tools/ray.cpp)
target_link_libraries(ray PRIVATE raycore)

add_executable(ray2tga tools/ray2tga.cpp)
target_link_libraries(ray2tga PRIVATE rayfile)

add_executable(raysplit tools/raysplit.cpp)
target_link_libraries(raysplit PRIVATE rayfile)

# rayview needs SDL2: prefer SDL2's CMake config, fall back to pkg-config, and
# skip the target when neither finds it.
find_package(SDL2 CONFIG QUIET)
if(TARGET SDL2::SDL2)
    set(RAY_SDL2 SDL2::SDL2)
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SDL2 QUIET IMPORTED_TARGET sdl2)
        if(TARGET PkgConfig::SDL2)
            set(RAY_SDL2 PkgConfig::SDL2)
        endif()
    endif()
endif()
if(RAY_SDL2)
    add_executable(rayview tools/rayview.cpp)
    target_link_libraries(rayview PRIVATE rayfile ${RAY_SDL2} Threads::Threads)
else()
    message(STATUS "SDL2 not found; rayview will not be built")
endif()

if(RAY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
