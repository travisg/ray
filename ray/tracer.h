/*
 * Copyright (c) 2008-2011 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include <cstdint>
#include <libvec/vector3.h>
#include <optional>
#include <ray/framebuffer.h>
#include <ray/ray.h>
#include <ray/rng.h>
#include <ray/trace_master.h>

class scene;

// How the image is rendered, as opposed to what is in it.
struct render_settings {
    // hemisphere rays per primary hit for indirect light; 0 keeps the constant ambient term
    int gi_samples = 0;
    // how many times indirect light may bounce; each bounce after the first follows one ray
    int bounces = 1;
    // every pixel's random sequence is derived from this, the pass and its coordinates
    uint32_t seed = 1;
    // which render pass this is; each pass gets fresh random sequences
    int pass = 0;
    // offset each primary ray randomly within its pixel (anti-aliasing over passes)
    bool jitter = false;
};

// One per worker thread: pulls tiles from the trace_master and casts the rays.
class tracer {
  public:
    tracer(const framebuffer &surface, const scene &s, trace_master &master,
           render_settings settings = {});

    void trace();
    // nullopt when the ray leaves the scene or the recursion limit is hit
    std::optional<colorf> cast(const ray &r);

    const render_settings &settings() const { return settings_; }
    // how many casts deep the shader asking is: 1 at a primary hit
    int depth() const { return depth_; }
    // uniform in [0, 1), from the current pixel's sequence
    double random() { return rng_.next_double(); }

  private:
    const framebuffer &surface_;
    const scene &scene_;
    trace_master &master_;
    const render_settings settings_;
    rng rng_{ 0 };

    int depth_ = 0;

    static constexpr int kMaxTraceDepth = 16;
};
