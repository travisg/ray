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
#include <algorithm>
#include <optional>
#include <ray/lights/simple_light.h>
#include <ray/ray.h>
#include <ray/sampling.h>
#include <ray/scene.h>
#include <ray/shaders/default_shader.h>
#include <ray/tracer.h>

using libvec::vector3d;

namespace {

// Secondary rays start this far above the surface along its normal.
constexpr double kBias = 0.001;

} // namespace

// Light reaching the point from other surfaces and the sky: the average over cosine-weighted
// hemisphere rays of what each one sees, times the albedo. A ray that escapes sees the
// scene's ambient colour as a uniform sky; one that lands gets that surface's direct light
// and, while bounces remain, its own single continuation ray, so a path never branches
// again after the primary hit. The primary hit's rays take one point per cell of a grid
// over the unit square, so no part of the hemisphere is skipped or sampled twice; the point
// in a cell is the pass's Halton point as that cell sees it (sequence_point), so over the
// passes each cell fills in evenly too and gi x passes rays behave like one large grid
// however they are split.
colorf default_shader::indirect_light(const shader_args &args, const vector3d &origin) const {
    const render_settings &settings = args.tracer.settings();
    const bool primary = args.tracer.depth() == 1;
    const strata grid = stratify(primary ? settings.gi_samples : 1);
    const auto pass = static_cast<uint32_t>(settings.pass);
    colorf sum;
    for (int i = 0; i < grid.x; i++) {
        for (int j = 0; j < grid.y; j++) {
            // two draws in a fixed order: as arguments their order would be the compiler's
            double u1, u2;
            if (primary) {
                u1 = (i + sequence_point(pass, 2, args.tracer.pixel_random())) / grid.x;
                u2 = (j + sequence_point(pass, 3, args.tracer.pixel_random())) / grid.y;
            } else {
                u1 = args.tracer.random();
                u2 = args.tracer.random();
            }
            const vector3d dir = cosine_hemisphere(args.hit.normal, u1, u2);
            sum += args.tracer.cast(ray(origin, dir)).value_or(args.scene.ambient_light());
        }
    }
    return (sum * (1.0 / grid.count())) * diffuse_color_;
}

colorf default_shader::run(const shader_args &args) const {
    const hit &h = args.hit;
    const vector3d origin = h.pos + h.normal * kBias;
    const render_settings &settings = args.tracer.settings();

    colorf color;
    if (settings.gi_samples == 0) {
        color = args.scene.ambient_light();
    } else if (args.tracer.depth() <= settings.bounces) {
        color = indirect_light(args, origin);
    }

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
