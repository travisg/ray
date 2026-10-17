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
#include <ray/cpp.h>
#include <ray/hit.h>
#include <ray/shaders/shader.h>
#include <utility>

struct ray;

class drawable {
  public:
    virtual ~drawable() = default;
    DISALLOW_COPY_ASSIGN_AND_MOVE(drawable);

    virtual int prepare() { return 0; }

    // ray.dir must be unit length for hit::t to be a distance
    virtual std::optional<hit> intersect(const ray &r) const = 0;

    void set_shader(shader_ptr s) { shader_ = std::move(s); }
    const shader_ptr &shader() const { return shader_; }

  protected:
    drawable() = default;

  private:
    shader_ptr shader_;
};
