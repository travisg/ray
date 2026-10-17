# ray

Small multithreaded CPU ray tracer (C++, GNU make) plus three helper tools that
consume its streaming `.ray` output. No tests exist; the check is "it builds and
`./doit` shows a picture".

## Build

```sh
make -j$(nproc)          # all four targets: ray rayview ray2tga raysplit
make ray.do              # one target (the .do suffix is the per-target goal)
make clean               # objects/deps for all targets
make spotless            # rm -rf build-*
NOECHO= make             # echo the compiler command lines
WITH_LTO=true make       # -flto
WITH_PROFILE=1 make      # -pg
```

- Objects go in `build-<target>/`; the top-level `ray`, `rayview`, `ray2tga`,
  `raysplit` are symlinks the build creates. Each link also gets `.lst` /
  `.debug.lst` objdump listings.
- `compile.mk` invokes `$(CC)` (not `$(CXX)`) to compile `.cpp` files and link,
  relying on `-lstdc++` in `LDLIBS`. `makefile` includes `compile.mk` once per
  target without resetting `LDLIBS`, so later targets inherit earlier libraries.
- `libvec` is a symlink to `../libvec`, a sibling checkout with its own git repo.
  The build fails without it. Don't edit it from here unless the task is about it.
- External deps: `boost_program_options`, `SDL2` (rayview only), pthreads.
- `tinyxml/` is compiled into `ray` but nothing references it yet.
- `testread.cpp`, `DisplayWindow.cpp`, `DisplayWindow.h`, and `sdl.h` are legacy
  files (SDL 1.2 era) not compiled in any target.
- Formatting: `./codestyle <files>` runs astyle (4-space indent, `-A4`). Match
  the existing style by hand if astyle is missing.

## Run

```sh
./ray [--xres 800] [--yres 600] [-c 8] [-o out.ray] [-r 1]  # -r 1 = random chunk order
./rayview            # opens ./out.ray (hard-coded) and follows it while ray writes
./doit               # ray at 3200x2400 in the background, then rayview
./ray2tga out.ray out.tga
./raysplit out.ray <chunkx> <chunky>   # mmaps to <name>.float, writes tiles
```

- Note on `-r` (`main.cpp`): `bool random` is uninitialized on the stack and
  configured with `implicit_value(false)` (no `default_value`), so bare `-r`
  sets `random = false` and `-r 1` / `--random=1` is required to enable
  `TraceMasterRandom`.
- `rayview` tails the file: short reads sleep and retry until `TYPE_EOF`, so
  start it any time after `ray` has written the header. Closing the window or
  pressing `Esc` while `ray` is still rendering blocks in `reader.join()` until
  `TYPE_EOF` is written. Renders `<= 2048x1600` open a 1:1 window; larger
  renders open an `800x600` resizable window and scale the full-res streaming
  texture to fit.

## Architecture

Everything includes from the repo root (`-I.`), so headers are written as
`<drawables/Sphere.h>`, `<libvec/Vector3.h>`, `<Ray.h>`. Each subdirectory
has a `rules.mk` that appends to `OBJS` via `GET_LOCAL_DIR`; a new source file
in a subdirectory goes there, while top-level sources go in `makefile`.

**Render pipeline (`main.cpp`)**

1. `RenderSurfaceFile` opens the output via `RayFile::OpenWrite`.
2. A `TraceMaster` (`TraceMasterSimple` = row-major 16x16 `WORKUNITSIZE` chunks,
   `TraceMasterRandom` = random unissued chunks via bitmap + `lrand48()`) owns
   the work queue. `GetWorkUnit` fills a `TraceWorkUnit` rectangle;
   `ReturnWorkUnit` copies its `colorf` result buffer into the surface under
   `m_Lock` and calls `SetDone` after the last unit comes back.
3. `--cpus` `std::thread`s each run a `Tracer`, which loops
   `GetWorkUnit` → trace every pixel → `ReturnWorkUnit` until it gets `< 0`.
4. `main` blocks in `TraceMaster::WaitForDone`, then joins the threads.

**Ray casting (`Tracer.cpp`)**

- `Tracer::Trace` builds the camera basis from `Scene::GetCamera/GetTarget`
  and casts one primary ray per pixel (no multisampling).
- `Tracer::Cast` asks `Scene::Intersect` for the nearest `Drawable` (linear
  scan over all drawables using 3-arg `Intersect(ray, pos, normal)`, no
  acceleration structure), re-intersects the winner for position/normal, then
  calls the drawable's `Shader::Run(ShaderArgs)`. Shaders recurse by calling
  `args.tracer->Cast` (reflection in `DefaultShader`); `m_Depth` caps recursion
  at `kMaxTraceDepth` (16).
- Shadow rays use `Scene::DoesIntersect(ray, maxDistance)` (which also calls
  3-arg `d->Intersect(ray, pos, normal)`). `ray.light = true` is set on shadow
  rays in `DefaultShader`, though nothing in active code reads it.
- Avoid the 1-arg `Scene::DoesIntersect(ray)` / `Drawable::Intersect(ray)`
  overload: `MeshDrawable::Intersect(ray)` is a stub that always returns `false`,
  and `Plane`/`Triangle` 1-arg `Intersect(ray)` only checks `Dot(dir, normal) <= 0`
  (ignoring `t < 0` and triangle bounds; `Plane`/`Triangle` 3-arg `Intersect`
  also omits a `t < 0` check).

**Scene (`Scene.cpp`)**

The scene is hard-coded in the `Scene` constructor behind `#if 0` / `#if 1` and
`if (0)` / `if (1)` blocks; flip those to switch scenes (note: `resources/lerp.obj`
referenced in the last `if (0)` block does not exist). There is no scene-file
loader. Every drawable must have `Prepare()` called before rendering; for
`MeshDrawable`, `Prepare()` computes `m_BSphere` (omitting it leaves radius 0).
Meshes are loaded with `LoadMesh("resources/*.obj")` → `obj_load` → `Geometry`,
wrapped in a `MeshDrawable` with a bounding-sphere pre-test. Lights are
`SimpleLight` point lights only (`m_Falloff` is stored but unused by shaders);
`lights/Light.h` is an empty base.

**Pluggable pieces**

- `drawables/`: `Drawable` interface = `Prepare()`, two `Intersect` overloads,
  and a `ShaderPtr`. Implementations: `Sphere`, `Plane`, `Triangle`,
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

Append-only stream: `RayHeader {magic 'RAY1', width, height}` then packets of
`uint32 type` followed by `RayDataPixel`, `RayDataPixelRun` (+ `length`
float triples) or `TYPE_EOF`. `RayFile::Parse` and `ReadIntoMmap` are the
readers used by `ray2tga`/`raysplit` (note: `ReadIntoMmap` has a bug where it
calls `munmap(buf, *len)` on `float **buf` instead of `*buf`, which fails
harmlessly on Linux so the mapping stays valid for `raysplit`). `rayview`
parses the stream itself so it can render while the file is still growing.
Changing the packet layout means updating `rayview.cpp` too.
