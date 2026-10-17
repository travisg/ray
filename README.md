# ray

A small multithreaded CPU ray tracer, written for fun. Spheres, planes,
triangles and Wavefront `.obj` meshes, point lights with shadows, mirror
reflections, optional bounced light, and a handful of built-in scenes.

It writes a PNG, or a float image (`.pfm`), when the render ends, and can
push finished tiles to a running [tev](https://github.com/Tom94/tev) viewer
so you watch the picture fill in tile by tile.

## Build

Needs a C++20 compiler and CMake 3.20+. The live view needs tev running.

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build
```

`cmake --list-presets` shows the clang, LTO and `-march=native` variants;
`cmake --preset clang-lto && cmake --build --preset clang-lto` is one of them.

## Use

```sh
build/ray --list-scenes
build/ray --scene spheres -x 1280 -y 960              # render to out.png
build/ray --scene spheres -o out.pfm                  # or keep the floats
build/ray --scene spheres --display localhost         # watch in tev while it renders
build/ray --scene cornell --gi 64 --bounces 3         # bounced light, Monte Carlo style
build/ray --scene cornell --gi 16 --passes 0 --display localhost   # refine in tev until Ctrl-C
./doit                                                # the same at 3200x2400
```

`build/ray --help` lists the options. `AGENTS.md` describes the code.

MIT licensed; see `LICENSE`.
