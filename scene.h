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

#include "color.h"
#include <cpp.h>
#include <drawables/drawable.h>
#include <hit.h>
#include <libvec/vector3.h>
#include <lights/simple_light.h>
#include <memory>
#include <optional>
#include <vector>

struct ray;

// Hits closer than this to a ray origin are the surface the ray left and are
// ignored, as are occluders this close to the end of a shadow ray.
inline constexpr double kMinT = 1e-6;

// Drawables, point lights, ambient light and a camera. Built by scene_library.
class scene {
  public:
    scene() = default;
    DISALLOW_COPY_ASSIGN_AND_MOVE(scene);

    void add_drawable(std::unique_ptr<drawable> d);
    void add_light(const simple_light &l);
    void set_ambient_light(const colorf &c) { ambient_light_ = c; }
    void set_camera(const libvec::vector3d &pos, const libvec::vector3d &target);
    // drawable::prepare on every drawable; run once after the last add_drawable
    void prepare();

    // nearest hit with t > kMinT, with hit::drawable set
    std::optional<hit> intersect(const ray &r) const;
    // true for any hit with kMinT < t < maxDistance - kMinT
    bool does_intersect(const ray &r, double max_distance) const;

    const std::vector<simple_light> &lights() const { return lights_; }
    colorf ambient_light() const { return ambient_light_; }

    const libvec::vector3d &camera() const { return camera_; }
    const libvec::vector3d &target() const { return target_; }

  private:
    std::vector<std::unique_ptr<drawable>> drawables_;
    std::vector<simple_light> lights_;
    colorf ambient_light_;

    libvec::vector3d camera_;
    libvec::vector3d target_;
};
