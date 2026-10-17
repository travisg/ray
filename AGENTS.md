# ray

Small multithreaded CPU ray tracer (C++20, CMake) plus three helper tools that
consume its streaming `.ray` output. `ctest` covers the vector math,
intersections, the tracer, the `.ray` reader/writer, the PFM writer, the tev
client, the OBJ loader and a golden render of every scene; `./doit` is the
eyeball check.

## Build

```sh
cmake -S . -B build -G Ninja           # configure; RelWithDebInfo (-O2 -g, asserts on)
cmake --build build                    # ray, ray2tga, raysplit, rayview (if SDL2 is found)
cmake --build build --target rayview   # one target
cmake --build build -v                 # echo the compiler command lines
cmake --build build --target clean     # objects only; rm -rf build for a full reset
```

Configure-time options, passed as `-D<name>=ON`: `RAY_NATIVE_ARCH`
(-march=native), `RAY_LTO`, `RAY_PROFILE` (-pg), `RAY_WERROR`, `RAY_BUILD_TESTS`
(default ON). `-DCMAKE_BUILD_TYPE=Debug` for -O0 -g.

- Binaries land in `build/`; nothing is written to the source tree.
  `build/compile_commands.json` is generated and `.clangd` points clangd at it.
- Sources are listed explicitly in `CMakeLists.txt`: `raycore` is the renderer,
  `rayfile` the `.ray` reader/writer the tools share. A new source file goes there.
- Dependencies: a C++20 compiler, CMake >= 3.20, pthreads, SDL2 (`rayview` only;
  missing SDL2 skips that target).
  macOS: `-DCMAKE_PREFIX_PATH=/opt/local` or `/opt/homebrew` if SDL2 isn't found.
- Portability: the tree must build with Apple's libc++, which lags on
  `std::jthread`, `std::format` and floating-point `std::from_chars`; use
  `std::thread` with explicit joins, `snprintf`/`std::to_string` and `strtof`.
- `libvec/` is the vendored, header-only vector math (`vector3.h`).
- Style: `STYLE.md` (snake_case types and functions, `name_` members, `kName`
  constants, class layout). `./codestyle <files>` runs `clang-format -i` with
  the repo's `.clang-format` (LLVM base, 4-space indent, attached braces,
  braces on every `if`/`for` body, 100 columns, sorted includes).
- Lint: `.clang-tidy` (advisory, `WarningsAsErrors` off) is picked up by clangd;
  `run-clang-tidy -p build $(git ls-files '*.cpp')` runs it over the tree.

## Test

```sh
ctest --test-dir build --output-on-failure    # all
ctest --test-dir build -R test_rayfile        # one (or run build/tests/test_rayfile)
ctest --test-dir build -LE slow               # skip the blob golden render
```

Tests are plain executables in `tests/` built on `tests/check.h`; CTest runs
them. `golden_render_<scene>` renders a scene at 64x48 single-threaded and
`golden_compare_<scene>` checks every channel against `tests/golden/<scene>.ray`
within 1e-3. When a change intentionally alters output, re-baseline in the same
commit:

```sh
ctest --test-dir build -R golden_render
cp build/tests/golden-<scene>.ray tests/golden/<scene>.ray
```

## Run

```sh
build/ray [-x 800] [-y 600] [-c N] [-o out.ray|out.pfm] [-s SCENE] [-r] [--seed N]
          [--display HOST[:PORT]]
build/ray --list-scenes
build/rayview [file.ray]     # default out.ray; follows the file while ray writes it
./doit               # ray at 3200x2400 in the background, then rayview
build/ray2tga out.ray out.tga
build/raysplit out.ray <chunkx> <chunky>   # writes <basename>-<x>.<y> tiles in the cwd
```

- `-o` picks the output by its extension: `.ray` streams tiles into the file as
  they finish (what `rayview` follows), `.pfm` keeps the image in a
  `framebuffer` and writes a portable float map when the render ends.
- `--display HOST[:PORT]` pushes tiles to a running tev viewer
  (https://github.com/Tom94/tev, port 14158 by default) as they finish. tev is
  the server: if nothing is listening the render goes on without it, retries
  the connection once a second, and sends the whole image so far when a viewer
  appears. The image is named after the output file, so a re-render replaces it.
- `rayview` polls the file every 100 ms until it sees `ray_data_type::eof`, waiting up
  to 10 s for the header, so it can start before or after `ray`. Esc or closing
  the window exits immediately. Renders up to 2048x1600 open a 1:1 window;
  larger ones open 800x600 and scale the full-resolution texture to fit.

## Layout

- The root holds only the build files, scripts and docs.
- `ray/`: the renderer library: `scene*`, `scene_library*`, `tracer*`,
  `trace_master*`, `render_surface*`, `framebuffer*`, `pfm*`, `tev_display*`,
  `ray_file*` (the `.ray` format), `hit.h`,
  `ray.h`, `color.h`, `bounding_sphere.h`, `cpp.h` (the `DISALLOW_*` class
  macros), and `drawables/`, `shaders/`, `lights/`, `mesh/` (geometry
  containers and the OBJ loader).
- `tools/`: the programs: `ray.cpp` (the renderer's CLI), `rayview.cpp`,
  `ray2tga.cpp`, `raysplit.cpp`.
- `libvec/` (vector math), `tests/` (with `golden/` references), `resources/`
  (`.obj` assets).

## Architecture

Everything includes from the repo root (`-I.`), so headers are written as
`<ray/drawables/sphere.h>`, `<libvec/vector3.h>`, `<ray/ray.h>`.

**Render pipeline (`tools/ray.cpp`)**

1. The output is a `render_surface`: `render_surface_file` streams into a
   `.ray` file via `ray_file::open_write`, or a `framebuffer` holds float RGB
   in memory and `write_pfm` saves it once the threads are joined.
2. A `trace_master` owns the tile list (`kWorkUnitSize` = 16 pixel squares,
   row-major, or shuffled with `--seed` when `--random`). `get_work_unit` fills a
   `trace_work_unit` rectangle; `return_work_unit` writes its `colorf` rows to the
   surface under `lock_` (the only thing serializing `ray_file`) and signals
   done when the last tile comes back.
3. `--cpus` `std::thread`s each run a `tracer`, which loops
   `get_work_unit` → trace every pixel → `return_work_unit` until it returns false.
4. `main` blocks in `trace_master::wait_for_done`, then joins the threads.
5. With `--display`, a `tee_surface` also feeds a `framebuffer`, and
   `display_thread` sends its dirty rectangle (`framebuffer::take_dirty`) to a
   `tev_display` every 100 ms, with one last pass after the workers are joined.

**Ray casting (`ray/tracer.cpp`)**

- `tracer::trace` builds the camera basis from `scene::camera/target`
  with +z up (+y when looking along z) and casts one primary ray per pixel
  through an image plane one unit ahead spanning [-1, 1] across its width
  (about a 90° horizontal field of view; no multisampling).
- `tracer::cast` asks `scene::intersect` for the nearest `hit` (linear scan
  over all drawables, nearest by `t`, no acceleration structure), then calls
  the hit drawable's `shader::run(shader_args)`. Shaders recurse by calling
  `args.tracer.cast` (reflection in `default_shader`); `depth_` caps recursion
  at `kMaxTraceDepth` (16).
- Shadow rays use `scene::does_intersect(ray, max_distance)`, true for any hit
  with `kMinT < t < maxDistance - kMinT`, so a light sitting exactly on a
  surface does not shadow it.
- `drawable::intersect(ray)` returns `std::optional<hit>` (`t`, `pos`,
  `normal`); `ray.dir` must be unit length for `t` to be a distance. Planes
  and triangles cull back faces; spheres hit from inside return the far side
  with the normal flipped to face the ray. `scene` ignores hits with
  `t <= kMinT` (1e-6) so a ray leaving a surface never re-hits it.

**Scene (`ray/scene.cpp`, `ray/scene_library.cpp`)**

`scene` is a container: owned drawables, point lights, ambient colour and a
camera position/target. Scenes are built by name in `ray/scene_library.cpp`, one
`Build*` function per entry in the `kScenes` table; `--list-scenes` prints
them and `--scene NAME` picks one (default `default`). `build_scene` runs
`scene::prepare()` once at the end, which calls `drawable::prepare()` on every
drawable (a `mesh_drawable` computes its bounding sphere there). Mesh scenes
load `resources/*.obj` relative to the working directory via `load_mesh` →
`load_obj` → `geometry`. The `spheres` scene draws from the `--seed` RNG.
Lights are `simple_light` point lights only (`falloff_` is stored but unused
by shaders).

**Pluggable pieces**

- `drawables/`: `drawable` interface = `prepare()`, `intersect()` and a
  `shader_ptr`. Implementations: `sphere`, `plane`, `triangle`,
  `mesh_drawable` (in `ray/drawables/mesh_drawable.h`, distinct from `ray/mesh/mesh.h`).
  `mesh_drawable` supports triangle and quad faces, interpolating the vertex
  normals the loader requires.
- `shaders/`: `shader::run(const shader_args&) -> colorf`. `default_shader`
  does ambient + clamped diffuse + shadows + mirror reflection, starting its
  secondary rays `kBias` (0.001) above the surface; `debug_shader` supports
  `mode::normal` and `mode::depth`.
- `mesh/`: `geometry` (shared vertex/normal/tangent/uv pools) owning `mesh` →
  `surface` index lists; `obj_loader` is the only importer.
- `ray/color.h`: `colorf` is double RGB (unclamped, additive); `color32` clamps to
  bytes (used directly as `b, g, r` by `convert_to_tga` and via packed `rgbx()`
  by `rayview`).

**`.ray` file format (`ray/ray_file.h`)**

Append-only stream: `ray_header {magic kRayHeaderMagic ('RAY1'), width, height}`
then packets of `uint32 type` followed by `ray_data_pixel`, `ray_data_pixel_run`
(+ `length` float triples) or `ray_data_type::eof`. `ray_stream_reader` parses it
incrementally (`need_data` at the end of the data so far, `eof` after the EOF
packet) and `ray_file::parse`/`read_into_buffer`, used by `ray2tga` and
`raysplit`, are built on it, as is `rayview`, which keeps pumping the reader
while the file grows.
