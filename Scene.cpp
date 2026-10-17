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
#include <Ray.h>
#include <Scene.h>
#include <utility>

void Scene::AddDrawable(std::unique_ptr<Drawable> d) {
    m_DrawableList.push_back(std::move(d));
}

void Scene::AddLight(const SimpleLight &l) {
    m_SimpleLightList.push_back(l);
}

void Scene::SetCamera(const Libvec::Vector3d &pos, const Libvec::Vector3d &target) {
    m_Camera = pos;
    m_Target = target;
}

void Scene::Prepare() {
    for (auto &d : m_DrawableList) {
        d->Prepare();
    }
}

std::optional<Hit> Scene::Intersect(const Ray &ray) const {
    std::optional<Hit> best;
    for (const auto &d : m_DrawableList) {
        std::optional<Hit> hit = d->Intersect(ray);
        if (hit && hit->t > kMinT && (!best || hit->t < best->t)) {
            best = hit;
            best->drawable = d.get();
        }
    }
    return best;
}

bool Scene::DoesIntersect(const Ray &ray, double maxDistance) const {
    for (const auto &d : m_DrawableList) {
        std::optional<Hit> hit = d->Intersect(ray);
        if (hit && hit->t > kMinT && hit->t < maxDistance - kMinT) {
            return true;
        }
    }
    return false;
}
