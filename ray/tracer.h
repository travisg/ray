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

#include <libvec/vector3.h>
#include <optional>
#include <ray/framebuffer.h>
#include <ray/ray.h>
#include <ray/trace_master.h>

class scene;

// One per worker thread: pulls tiles from the trace_master and casts the rays.
class tracer {
  public:
    tracer(const framebuffer &surface, const scene &s, trace_master &master);

    void trace();
    // nullopt when the ray leaves the scene or the recursion limit is hit
    std::optional<colorf> cast(const ray &r);

  private:
    const framebuffer &surface_;
    const scene &scene_;
    trace_master &master_;

    int depth_ = 0;

    static constexpr int kMaxTraceDepth = 16;
};
