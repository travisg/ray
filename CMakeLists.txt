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

# Renderer core: scene, tracing, work distribution, drawables, shaders, meshes.
add_library(raycore STATIC
    ray/scene.cpp
    ray/scene_library.cpp
    ray/tracer.cpp
    ray/trace_master.cpp
    ray/bvh.cpp
    ray/framebuffer.cpp
    ray/pfm.cpp
    ray/png.cpp
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
target_include_directories(raycore PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(raycore PUBLIC ray_flags Threads::Threads)
# vendored headers; a system directory so their warnings stay quiet
target_include_directories(raycore SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/external)

add_executable(ray tools/ray.cpp)
target_link_libraries(ray PRIVATE raycore)


if(RAY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
