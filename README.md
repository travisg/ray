# ray

A small multithreaded CPU ray tracer, written for fun, plus three tools that
work on its output. Spheres, planes, triangles and Wavefront `.obj` meshes,
point lights with shadows, mirror reflections, and a handful of built-in
scenes.

The renderer streams finished tiles into an append-only `.ray` file as they
complete, and `rayview` follows that file while it grows, so you watch the
picture fill in tile by tile. It can also push tiles to a running
[tev](https://github.com/Tom94/tev) viewer, and write a plain float image
(`.pfm`) when the render ends.

## Build

Needs a C++20 compiler, CMake 3.20+ and, for the viewer, SDL2.

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build
```

## Use

```sh
build/ray --list-scenes
build/ray --scene spheres -x 1280 -y 960 -o out.ray   # render
build/ray --scene spheres -o out.pfm                  # or a float image at the end
build/ray --scene spheres -o out.pfm --display localhost   # watch in tev while it renders
build/rayview out.ray                                 # watch it arrive
build/ray2tga out.ray out.tga                         # convert when done
./doit                                                # both, at 3200x2400
```

`build/ray --help` lists the options. `AGENTS.md` describes the code.

MIT licensed; see `LICENSE`.
