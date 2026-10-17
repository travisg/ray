# ray

Small multithreaded CPU ray tracer (C++20, CMake) plus three helper tools that
consume its streaming `.ray` output. `ctest` covers the vector math, the `.ray`
reader/writer, the OBJ loader and a golden render; `./doit` is the eyeball check.

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
- `libvec/` is the vendored, header-only vector math (`Vector3.h`, `Random.h`).
- Formatting: `./codestyle <files>` runs `clang-format -i` with the repo's
  `.clang-format` (LLVM base, 4-space indent, attached braces, braces on every
  `if`/`for` body, 100 columns, sorted includes).

## Test

```sh
ctest --test-dir build --output-on-failure    # all
ctest --test-dir build -R test_rayfile        # one (or run build/tests/test_rayfile)
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
build/ray [-x 800] [-y 600] [-c N] [-o out.ray] [-s SCENE] [-r] [--seed N]
build/ray --list-scenes
build/rayview [file.ray]     # default out.ray; follows the file while ray writes it
./doit               # ray at 3200x2400 in the background, then rayview
build/ray2tga out.ray out.tga
build/raysplit out.ray <chunkx> <chunky>   # writes <basename>-<x>.<y> tiles in the cwd
```

- `rayview` polls the file every 100 ms until it sees `TYPE_EOF`, waiting up
  to 10 s for the header, so it can start before or after `ray`. Esc or closing
  the window exits immediately. Renders up to 2048x1600 open a 1:1 window;
  larger ones open 800x600 and scale the full-resolution texture to fit.

## Architecture

Everything includes from the repo root (`-I.`), so headers are written as
`<drawables/Sphere.h>`, `<libvec/Vector3.h>`, `<Ray.h>`.

**Render pipeline (`main.cpp`)**

1. `RenderSurfaceFile` opens the output via `RayFile::OpenWrite`.
2. A `TraceMaster` owns the tile list (`kWorkUnitSize` = 16 pixel squares,
   row-major, or shuffled with `--seed` when `--random`). `GetWorkUnit` fills a
   `TraceWorkUnit` rectangle; `ReturnWorkUnit` writes its `colorf` rows to the
   surface under `m_Lock` (the only thing serializing `RayFile`) and signals
   done when the last tile comes back.
3. `--cpus` `std::thread`s each run a `Tracer`, which loops
   `GetWorkUnit` → trace every pixel → `ReturnWorkUnit` until it returns false.
4. `main` blocks in `TraceMaster::WaitForDone`, then joins the threads.

**Ray casting (`Tracer.cpp`)**

- `Tracer::Trace` builds the camera basis from `Scene::GetCamera/GetTarget`
  and casts one primary ray per pixel (no multisampling).
- `Tracer::Cast` asks `Scene::Intersect` for the nearest `Hit` (linear scan
  over all drawables, nearest by `t`, no acceleration structure), then calls
  the hit drawable's `Shader::Run(ShaderArgs)`. Shaders recurse by calling
  `args.tracer.Cast` (reflection in `DefaultShader`); `m_Depth` caps recursion
  at `kMaxTraceDepth` (16).
- Shadow rays use `Scene::DoesIntersect(ray, maxDistance)`, true for any hit
  with `t < maxDistance`.
- `Drawable::Intersect(ray)` returns `std::optional<Hit>` (`t`, `pos`,
  `normal`); `ray.dir` must be unit length for `t` to be a distance. Planes
  and triangles cull back faces; spheres hit from inside return the far side
  with the normal flipped to face the ray. `Scene` ignores hits with
  `t <= kMinT` (1e-6) so a ray leaving a surface never re-hits it.

**Scene (`Scene.cpp`, `SceneLibrary.cpp`)**

`Scene` is a container: owned drawables, point lights, ambient colour and a
camera position/target. Scenes are built by name in `SceneLibrary.cpp`, one
`Build*` function per entry in the `kScenes` table; `--list-scenes` prints
them and `--scene NAME` picks one (default `default`). `BuildScene` runs
`Scene::Prepare()` once at the end, which calls `Drawable::Prepare()` on every
drawable (a `MeshDrawable` computes its bounding sphere there). Mesh scenes
load `resources/*.obj` relative to the working directory via `LoadMesh` →
`LoadObj` → `Geometry`. The `spheres` scene draws from the `--seed` RNG.
Lights are `SimpleLight` point lights only (`m_Falloff` is stored but unused
by shaders).

**Pluggable pieces**

- `drawables/`: `Drawable` interface = `Prepare()`, `Intersect()` and a
  `ShaderPtr`. Implementations: `Sphere`, `Plane`, `Triangle`,
  `MeshDrawable` (in `drawables/Mesh.h`, distinct from `mesh/Mesh.h`).
  `MeshDrawable` supports triangle (3-index) and quad (4-index) faces and
  requires vertex normals (`vn`) in the `.obj`.
- `shaders/`: `Shader::Run(const ShaderArgs&) -> colorf`. `DefaultShader`
  does ambient + diffuse + shadows + mirror reflection; `DebugShader` supports
  `MODE_NORMAL` and `MODE_DEPTH`.
- `mesh/`: `Geometry` (shared vertex/normal/tangent/uv pools) owning `Mesh` →
  `Surface` index lists; `OBJLoader` is the only importer.
- `Color.h`: `colorf` is double RGB (unclamped, additive); `color32` clamps to
  bytes (used directly as `b, g, r` by `ConvertToTGA` and via packed `rgbx()`
  by `rayview`).

**`.ray` file format (`RayFile.h`)**

Append-only stream: `RayHeader {magic kRayHeaderMagic ('RAY1'), width, height}`
then packets of `uint32 type` followed by `RayDataPixel`, `RayDataPixelRun`
(+ `length` float triples) or `TYPE_EOF`. `RayStreamReader` parses it
incrementally (`NeedData` at the end of the data so far, `Eof` after the EOF
packet) and `RayFile::Parse`/`ReadIntoBuffer`, used by `ray2tga` and
`raysplit`, are built on it, as is `rayview`, which keeps pumping the reader
while the file grows.
