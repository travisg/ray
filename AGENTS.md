# ray

Small multithreaded CPU ray tracer (C++20, CMake). It renders into an
in-memory framebuffer, writes a PNG or PFM at the end, and can push tiles to
a tev viewer as they finish. `ctest` covers the vector math, intersections,
the tracer, the bvh, the PNG and PFM writers, the tev client, the OBJ loader
and a golden render of every scene; `./doit` is the eyeball check.

## Build

```sh
cmake -S . -B build -G Ninja           # configure; RelWithDebInfo (-O2 -g, asserts on)
cmake --build build                    # ray and the tests
cmake --build build --target ray       # one target
cmake --build build -v                 # echo the compiler command lines
cmake --build build --target clean     # objects only; rm -rf build for a full reset
```

Configure-time options, passed as `-D<name>=ON`: `RAY_NATIVE_ARCH`
(-march=native), `RAY_LTO`, `RAY_PROFILE` (-pg), `RAY_WERROR`, `RAY_BUILD_TESTS`
(default ON). `-DCMAKE_BUILD_TYPE=Debug` for -O0 -g.

`CMakePresets.json` names the common combinations; each configures into its
own `build-<preset>/` (`default` is `build/`). `native` and `clang-native`
are the fast ones: -O3, LTO and -march=native, so their binaries only run on
this machine's CPU class.

```sh
cmake --list-presets                   # default, clang, lto, clang-lto, native, clang-native, debug
cmake --preset clang-lto               # configure build-clang-lto/
cmake --build --preset clang-lto
ctest --preset clang-lto
```

- Binaries land in `build/`; nothing is written to the source tree.
  `build/compile_commands.json` is generated and `.clangd` points clangd at it.
- Sources are listed explicitly in `CMakeLists.txt`: `raycore` is the renderer
  library, `ray` the program. A new source file goes there.
- Dependencies: a C++20 compiler, CMake >= 3.20, pthreads. The live view needs
  a tev viewer (https://github.com/Tom94/tev) running, not a library.
- Portability: the tree must build with Apple's libc++, which lags on
  `std::jthread`, `std::format` and floating-point `std::from_chars`; use
  `std::thread` with explicit joins, `snprintf`/`std::to_string` and `strtof`.
- `libvec/` is the vendored, header-only vector math (`vector3.h`);
  `external/` holds third-party code (`stb/stb_image_write.h` for PNG), on
  the include path as a system directory so its warnings stay quiet.
- Style: `STYLE.md` (snake_case types and functions, `name_` members, `kName`
  constants, class layout). `./codestyle <files>` runs `clang-format -i` with
  the repo's `.clang-format` (LLVM base, 4-space indent, attached braces,
  braces on every `if`/`for` body, 100 columns, sorted includes).
- Lint: `.clang-tidy` (advisory, `WarningsAsErrors` off) is picked up by clangd;
  `run-clang-tidy -p build $(git ls-files '*.cpp')` runs it over the tree.

## Test

```sh
ctest --test-dir build --output-on-failure    # all
ctest --test-dir build -R test_pfm            # one (or run build/tests/test_pfm)
```

Tests are plain executables in `tests/` built on `tests/check.h`; CTest runs
them. `golden_render_<scene>` renders a scene at 64x48 single-threaded and
`golden_compare_<scene>` checks every channel against `tests/golden/<scene>.pfm`
within 1e-3; the `-gi` variants render with `--gi 16`, so they also pin the
random sequence. When a change intentionally alters output, re-baseline in the
same commit:

```sh
ctest --test-dir build -R golden_render
cp build/tests/golden-<scene>.pfm tests/golden/<scene>.pfm
```

## Run

```sh
build/ray [-x 800] [-y 600] [-c N] [-o out.png|out.pfm] [-s SCENE] [-r] [--seed N]
          [--gi N] [--bounces N] [--passes N] [--display HOST[:PORT]]
build/ray --list-scenes
./doit [more ray options]    # 3200x2400, pushing tiles to a tev on localhost
```

- `-o` picks the output by its extension: `.png` (the default, 8-bit sRGB) or
  `.pfm` (the linear floats), written from the `framebuffer` when the render
  ends.
- `--gi N` turns on indirect light: N cosine-weighted hemisphere rays from every
  primary hit, one per cell of a grid of strata over the hemisphere (N rounds
  up to the grid, 64 is 8x8), each continued for `--bounces` bounces (default
  1) with one ray per bounce. Rays that escape see the scene's ambient colour
  as a uniform sky, which is also what the background shows. Cost grows with
  N; 64 is a usable preview, 256 is smooth. Off (the default) keeps the
  constant ambient term.
- `--passes N` renders the image N times, each pass with fresh random sequences
  and the primary ray jittered within its pixel, and keeps the running mean
  (`framebuffer::accumulate_run`), so noise falls and edges anti-alias as the
  passes go by; with `--display` the viewer shows it converging. `--passes 0`
  runs until Ctrl-C, which ends the render after the current pass and writes
  the image so far; a second Ctrl-C kills it.
- `--display HOST[:PORT]` pushes tiles to a running tev viewer
  (https://github.com/Tom94/tev, port 14158 by default) as they finish. tev is
  the server: if nothing is listening the render goes on without it, retries
  the connection once a second, and sends the whole image so far when a viewer
  appears. The image is named after the output file, so a re-render replaces it.

## Layout

- The root holds only the build files, scripts and docs.
- `ray/`: the renderer library: `scene*`, `scene_library*`, `tracer*`,
  `trace_master*`, `framebuffer*`, `png*`, `pfm*`, `tev_display*`, `hit.h`,
  `ray.h`, `color.h`, `aabb.h`, `bvh*`, `triangle_hit.h`, `rng.h`,
  `sampling.h`, `cpp.h` (the `DISALLOW_*` class macros), and `drawables/`, `shaders/`, `lights/`, `mesh/` (geometry
  containers and the OBJ loader).
- `tools/`: `ray.cpp`, the renderer's CLI.
- `libvec/` (vector math), `external/` (vendored third-party code), `tests/`
  (with `golden/` references), `resources/` (`.obj` assets).

## Architecture

Everything includes from the repo root (`-I.`), so headers are written as
`<ray/drawables/sphere.h>`, `<libvec/vector3.h>`, `<ray/ray.h>`.

**Render pipeline (`tools/ray.cpp`)**

1. A `framebuffer` holds the image as float RGB; `write_png` (sRGB encoded)
   or `write_pfm` saves it once the threads are joined.
2. A `trace_master` per pass owns the tile list (`kWorkUnitSize` = 16 pixel
   squares, row-major, or shuffled with `--seed` when `--random`).
   `get_work_unit` fills a `trace_work_unit` rectangle; `return_work_unit`
   folds its `colorf` rows into the framebuffer's running mean
   (`accumulate_run` with the pass number) under `lock_` and signals done when
   the last tile comes back.
3. `--cpus` `std::thread`s each run a `tracer`, which loops
   `get_work_unit` → trace every pixel → `return_work_unit` until it returns false.
4. `main` blocks in `trace_master::wait_for_done`, then joins the threads.
5. With `--display`, `display_thread` sends the framebuffer's dirty rectangle
   (`framebuffer::take_dirty`) to a `tev_display` every 100 ms, with one last
   pass after the workers are joined.

**Ray casting (`ray/tracer.cpp`)**

- `tracer::trace` builds the camera basis from `scene::camera/target`
  with +z up (+y when looking along z) and casts one primary ray per pixel
  through an image plane one unit ahead spanning [-1, 1] across its width
  (about a 90° horizontal field of view). Each pixel reseeds the tracer's
  PCG32 `rng` from `--seed`, the pass number and its coordinates, so a render
  is the same whichever thread drew which tile; on multi-pass renders the ray
  goes through the pass's 2D Halton point (`radical_inverse` in
  `ray/sampling.h`, bases 2 and 3), shifted by a per-pixel offset, within half
  a pixel of the usual ray, so the edge samples spread evenly over the pixel
  as passes accumulate.
  `render_settings` carries the `--gi`, `--bounces`, `--seed` and pass values
  to the shaders through `tracer::settings()`.
- `tracer::cast` asks `scene::intersect` for the nearest `hit` (a linear scan
  over the drawables, nearest by `t`; a `mesh_drawable` walks a `bvh` over its
  faces, see below), then calls
  the hit drawable's `shader::run(shader_args)`. Shaders recurse by calling
  `args.tracer.cast` (reflection in `default_shader`); `depth_` caps recursion
  at `kMaxTraceDepth` (16).
- Shadow rays use `scene::does_intersect(ray, max_distance)`, true for any hit
  with `kMinT < t < maxDistance - kMinT`, so a light sitting exactly on a
  surface does not shadow it.
- `drawable::intersect(ray)` returns `std::optional<hit>` (`t`, `pos`,
  `normal`); `ray.dir` must be unit length for `t` to be a distance. Planes
  and triangles cull back faces; spheres hit from inside return the far side
  with the normal flipped to face the ray. `triangle_hit.h` is the one
  ray/triangle test, shared by `triangle` and `mesh_drawable`; it accepts
  points `kEdgeSlop` outside an edge so a ray through an edge or vertex two
  triangles share cannot be rejected by both and fall through. `scene` ignores hits with
  `t <= kMinT` (1e-6) so a ray leaving a surface never re-hits it.

**Scene (`ray/scene.cpp`, `ray/scene_library.cpp`)**

`scene` is a container: owned drawables, point lights, ambient colour and a
camera position/target. Scenes are built by name in `ray/scene_library.cpp`, one
`Build*` function per entry in the `kScenes` table; `--list-scenes` prints
them and `--scene NAME` picks one (default `default`). `build_scene` runs
`scene::prepare()` once at the end, which calls `drawable::prepare()` on every
drawable (a `mesh_drawable` builds its bvh there). Mesh scenes
load `resources/*.obj` relative to the working directory via `load_mesh` →
`load_obj` → `geometry`; `load_mesh` takes a uniform scale and an offset,
applied to the positions by `geometry::transform`, which is how `map` fits
its 65536-unit heightfield into 16 units and `map-blob` lifts the blob above
it. The `spheres` scene draws from the `--seed` RNG.
Lights are `simple_light` point lights only (`falloff_` is stored but unused
by shaders).

**Pluggable pieces**

- `drawables/`: `drawable` interface = `prepare()`, `intersect()` and a
  `shader_ptr`. Implementations: `sphere`, `plane`, `triangle`,
  `mesh_drawable` (in `ray/drawables/mesh_drawable.h`, distinct from `ray/mesh/mesh.h`).
  `mesh_drawable` supports triangle and quad faces, interpolating the vertex
  normals the loader requires. Its `bvh` (`ray/bvh.h`) is a tree of `aabb`
  boxes over the faces, median split on the longest centroid axis, leaves of
  up to 4 faces; `intersect` walks it nearest-box-first with a stack, pruning
  boxes beyond the best hit, and ties at equal `t` go to the lower face
  number so the result matches a plain scan.
- `shaders/`: `shader::run(const shader_args&) -> colorf`. `default_shader`
  does ambient + clamped diffuse + shadows + mirror reflection, starting its
  secondary rays `kBias` (0.001) above the surface. With `gi_samples` set,
  `indirect_light` replaces the ambient term: `cosine_hemisphere`
  (`ray/sampling.h`) directions from one random point per cell of a
  `stratify(gi_samples)` grid, `tracer::cast` for what each sees, the sky
  for escapes, averaged and scaled by the albedo; the primary hit (depth 1)
  spends the grid and deeper hits one ray, up to `bounces`.
  `debug_shader` supports `mode::normal` and `mode::depth`.
- `mesh/`: `geometry` (shared vertex/normal/tangent/uv pools) owning `mesh` →
  `surface` index lists; `obj_loader` is the only importer.
- `ray/color.h`: `colorf` is double RGB, unclamped and additive; the
  framebuffer stores it as float and `write_png` clamps and sRGB-encodes.
