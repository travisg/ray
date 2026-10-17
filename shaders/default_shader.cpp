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
#include "default_shader.h"
#include <algorithm>
#include <lights/simple_light.h>
#include <optional>
#include <ray.h>
#include <scene.h>
#include <tracer.h>

using libvec::vector3d;

namespace {

// Secondary rays start this far above the surface along its normal.
constexpr double kBias = 0.001;

} // namespace

colorf default_shader::run(const shader_args &args) const {
    const hit &h = args.hit;
    const vector3d origin = h.pos + h.normal * kBias;
    colorf color = args.scene.ambient_light();

    for (const simple_light &l : args.scene.lights()) {
        // shadow ray toward the light
        vector3d tolight = l.position() - origin;
        const double distance = tolight.length();
        tolight.normalize();
        if (args.scene.does_intersect(ray(origin, tolight), distance)) {
            continue;
        }

        const double diffuse = std::max(0.0, dot(tolight, h.normal));
        color += (l.color() * diffuse) * diffuse_color_;
    }

    if (shinyness_ > 0.0) {
        // mirror reflection
        const double d = dot(args.ray.dir, h.normal);
        const ray reflected(origin, args.ray.dir - h.normal * (2.0 * d));

        color *= (1.0 - shinyness_);
        if (std::optional<colorf> reflection = args.tracer.cast(reflected)) {
            color += *reflection * shinyness_;
        }
    }

    return color;
}
