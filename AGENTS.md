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
- `libvec` is a symlink to `../libvec`, a sibling checkout with its own git repo.
  The build fails without it. Don't edit it from here unless the task is about it.
- External deps: `boost_program_options`, `SDL2` (rayview only), pthreads.
- `tinyxml/` is compiled into `ray` but nothing references it yet.
- `testread.cpp` is not in any target.
- Formatting: `./codestyle <files>` runs astyle (4-space indent, `-A4`). Match
  the existing style by hand if astyle is missing.

## Run

```sh
./ray [--xres 800] [--yres 600] [-c 8] [-o out.ray] [-r]   # -r = random chunk order
./rayview            # opens ./out.ray (hard-coded) and follows it while ray writes
./doit               # ray at 3200x2400 in the background, then rayview
./ray2tga out.ray out.tga
./raysplit out.ray <chunkx> <chunky>   # mmaps to <name>.float, writes tiles
```

`rayview` tails the file: short reads sleep and retry, so start it any time
after `ray` has written the header. The window is clamped to 2048x1600; larger
renders are scaled.

## Architecture

Everything includes from the repo root (`-I.`), so headers are written as
`<drawables/Sphere.h>`, `<libvec/Vector3.h>`, `<Ray.h>`. Each subdirectory
has a `rules.mk` that appends to `OBJS` via `GET_LOCAL_DIR`; a new source file
goes there, not in the top-level makefile.

**Render pipeline (`main.cpp`)**

1. `RenderSurfaceFile` opens the output via `RayFile::OpenWrite`.
2. A `TraceMaster` (`TraceMasterSimple` = row-major chunks, `TraceMasterRandom`
   = shuffled chunks) owns the work queue. `GetWorkUnit` fills a
   `TraceWorkUnit` rectangle; `ReturnWorkUnit` copies its `colorf` result
   buffer into the surface under `m_Lock` and calls `SetDone` after the last
   unit comes back.
3. `--cpus` `std::thread`s each run a `Tracer`, which loops
   `GetWorkUnit` → trace every pixel → `ReturnWorkUnit` until it gets `< 0`.
4. `main` blocks in `TraceMaster::WaitForDone`, then joins the threads.

**Ray casting (`Tracer.cpp`)**

- `Tracer::Trace` builds the camera basis from `Scene::GetCamera/GetTarget`
  and casts one primary ray per pixel (no multisampling).
- `Tracer::Cast` asks `Scene::Intersect` for the nearest `Drawable` (linear
  scan over all drawables, no acceleration structure), re-intersects it for
  position/normal, then calls the drawable's `Shader::Run(ShaderArgs)`.
  Shaders recurse by calling `args.tracer->Cast` (reflection in
  `DefaultShader`); `m_Depth` caps recursion at `kMaxTraceDepth` (16).
- Shadow rays use `Scene::DoesIntersect(ray, maxDistance)` with `ray.light`
  set.

**Scene (`Scene.cpp`)**

The scene is hard-coded in the `Scene` constructor behind `#if 0` / `#if 1`
blocks; flip those to switch scenes. There is no scene-file loader. Meshes are
loaded with `LoadMesh("resources/*.obj")` → `obj_load` → `Geometry`, wrapped
in a `MeshDrawable` with a bounding-sphere pre-test. Lights are
`SimpleLight` point lights only; `lights/Light.h` is an empty base.

**Pluggable pieces**

- `drawables/`: `Drawable` interface = two `Intersect` overloads + a
  `ShaderPtr`. Implementations: `Sphere`, `Plane`, `Triangle`, `MeshDrawable`.
- `shaders/`: `Shader::Run(const ShaderArgs&) -> colorf`. `DefaultShader`
  does ambient + diffuse + shadows + mirror reflection; `DebugShader` for
  normals.
- `mesh/`: `Geometry` (shared vertex/normal/uv pools) owning `Mesh` → `Surface`
  index lists; `OBJLoader` is the only importer.
- `Color.h`: `colorf` is double RGB (unclamped, additive); `color32` clamps to
  bytes and provides the packed `rgbx()` etc. used by rayview/TGA.

**`.ray` file format (`RayFile.h`)**

Append-only stream: `RayHeader {magic 'RAY1', width, height}` then packets of
`uint32 type` followed by `RayDataPixel`, `RayDataPixelRun` (+ `length`
float triples) or `TYPE_EOF`. `RayFile::Parse` and `ReadIntoMmap` are the
readers used by `ray2tga`/`raysplit`; `rayview` parses the stream itself so it
can render while the file is still growing. Changing the packet layout means
updating `rayview.cpp` too.
