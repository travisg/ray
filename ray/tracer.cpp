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

#include <ray/drawables/drawable.h>
#include <ray/ray.h>
#include <ray/scene.h>
#include <ray/shaders/shader.h>
#include <ray/tracer.h>

using libvec::vector3d;

tracer::tracer(const framebuffer &surface, const scene &s, trace_master &master,
               render_settings settings)
    : surface_(surface), scene_(s), master_(master), settings_(settings) {}

std::optional<colorf> tracer::cast(const ray &r) {
    if (depth_ == kMaxTraceDepth) {
        return std::nullopt;
    }

    std::optional<hit> hit = scene_.intersect(r);
    if (!hit) {
        return std::nullopt;
    }

    const shader_ptr &s = hit->drawable->shader();
    if (!s) {
        return colorf(0);
    }

    shader_args args{ scene_, *this, r, *hit };

    depth_++; // track the level of recursion
    colorf color = s->run(args);
    depth_--;
    return color;
}

void tracer::trace() {
    const int width = surface_.width();
    const int height = surface_.height();
    const vector3d camera = scene_.camera();

    // camera basis: forward toward the target, right and down across the image
    vector3d forward = scene_.target() - camera;
    forward.normalize();
    vector3d up(0, 0, 1);
    vector3d right = cross(forward, up);
    if (right.length_squared() < 1e-12) {
        // looking straight along z
        up = vector3d(0, 1, 0);
        right = cross(forward, up);
    }
    right.normalize();
    vector3d down = cross(forward, right);
    down.normalize();

    // the image plane sits one unit ahead and spans [-1, 1] across its width
    const double pixpitch = 2.0 / width;

    trace_work_unit unit;
    while (master_.get_work_unit(unit)) {
        for (int y = unit.starty; y <= unit.endy; y++) {
            for (int x = unit.startx; x <= unit.endx; x++) {
                // the pixel's own random sequence, so the image does not depend on which
                // thread drew it
                rng_ = rng(
                    mix64(mix64(settings_.seed ^ (static_cast<uint64_t>(settings_.pass) << 32)) ^
                          (static_cast<uint64_t>(y) << 32) ^ static_cast<uint64_t>(x)));

                // the ray goes through the pixel's reference point, or when jittering through
                // a random point within half a pixel of it, so the passes average to an image
                // centred on the same rays
                double px = x;
                double py = y;
                if (settings_.jitter) {
                    px += rng_.next_double() - 0.5;
                    py += rng_.next_double() - 0.5;
                }
                vector3d dir = forward + right * ((px - width / 2.0) * pixpitch) +
                               down * ((py - height / 2.0) * pixpitch);
                dir.normalize();

                // where the ray exits the world: the sky the indirect light comes from, or black
                // when there is no indirect light
                const colorf background =
                    settings_.gi_samples > 0 ? scene_.ambient_light() : colorf(0);
                unit.set_result(x, y, cast(ray(camera, dir)).value_or(background));
            }
        }

        master_.return_work_unit(unit);
    }
}
