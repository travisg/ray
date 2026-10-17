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
#include <ray/ray.h>
#include <ray/scene.h>
#include <utility>

void scene::add_drawable(std::unique_ptr<drawable> d) {
    drawables_.push_back(std::move(d));
}

void scene::add_light(const simple_light &l) {
    lights_.push_back(l);
}

void scene::set_camera(const libvec::vector3d &pos, const libvec::vector3d &target) {
    camera_ = pos;
    target_ = target;
}

void scene::prepare() {
    for (auto &d : drawables_) {
        d->prepare();
    }
}

std::optional<hit> scene::intersect(const ray &r) const {
    std::optional<hit> best;
    for (const auto &d : drawables_) {
        std::optional<hit> h = d->intersect(r);
        if (h && h->t > kMinT && (!best || h->t < best->t)) {
            best = h;
            best->drawable = d.get();
        }
    }
    return best;
}

bool scene::does_intersect(const ray &r, double max_distance) const {
    for (const auto &d : drawables_) {
        std::optional<hit> h = d->intersect(r);
        if (h && h->t > kMinT && h->t < max_distance - kMinT) {
            return true;
        }
    }
    return false;
}
