# TODO

Planned, roughly in order of interest:

- Texture mapping: the loader already keeps `vt` coordinates and `surface_index::uv_index`;
  `hit` needs a uv, `default_shader` an image lookup, and a PNG or PFM reader for the texture.
- A scene description file, so scenes are data rather than `build_*` functions in
  `scene_library.cpp`: drawables, shaders, lights, camera, mesh assets with scale and offset.
- SIMD: the hot paths are scalar `vector3<double>` work, one ray at a time. Real gains need
  float, structure-of-arrays data and a triangle test over several rays or several triangles
  at once; `-march=native` today only buys FMA and some packed arithmetic.
- GPU offload: the bvh and the triangle test are the parts worth moving first.

Smaller items noted along the way:

- Any-hit traversal for shadow rays, stopping at the first occluder instead of finding the
  nearest hit; roughly halves shadow cost.
- Russian roulette for path termination instead of the fixed `--bounces` cap.
- A tent or Gaussian pixel filter in place of the box the jitter gives.
- Flat shading as the fallback for meshes without normals, and OBJ smoothing groups.
- `simple_light::falloff` is stored but unused.
- A console on the running render process, in place of command line switches.
- A denoiser (Open Image Denoise) if low sample counts ever need to look finished.
