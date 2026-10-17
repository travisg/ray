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
#include <iostream>

#include <ray.h>
#include <scene.h>
#include <tracer.h>
#include <drawables/drawable.h>
#include <shaders/shader.h>

using Libvec::Vector3d;

Tracer::Tracer(const RenderSurface &surface, const Scene &scene, TraceMaster &tracemaster)
    : m_Surface(surface), m_Scene(scene), m_Master(tracemaster) {}

std::optional<colorf> Tracer::Cast(const Ray &ray) {
    if (m_Depth == kMaxTraceDepth) {
        return std::nullopt;
    }

    std::optional<Hit> hit = m_Scene.Intersect(ray);
    if (!hit) {
        return std::nullopt;
    }

    const ShaderPtr &s = hit->drawable->GetShader();
    if (!s) {
        return colorf(0);
    }

    ShaderArgs args{ m_Scene, *this, ray, *hit };

    m_Depth++; // track the level of recursion
    colorf color = s->Run(args);
    m_Depth--;
    return color;
}

void Tracer::Trace() {
    const int width = m_Surface.Width();
    const int height = m_Surface.Height();
    const Vector3d camera = m_Scene.GetCamera();

    // camera basis: forward toward the target, right and down across the image
    Vector3d forward = m_Scene.GetTarget() - camera;
    forward.Normalize();
    Vector3d up(0, 0, 1);
    Vector3d right = Cross(forward, up);
    if (right.LengthSquared() < 1e-12) {
        // looking straight along z
        up = Vector3d(0, 1, 0);
        right = Cross(forward, up);
    }
    right.Normalize();
    Vector3d down = Cross(forward, right);
    down.Normalize();

    // the image plane sits one unit ahead and spans [-1, 1] across its width
    const double pixpitch = 2.0 / width;

    TraceWorkUnit unit;
    while (m_Master.GetWorkUnit(unit)) {
        for (int y = unit.starty; y <= unit.endy; y++) {
            for (int x = unit.startx; x <= unit.endx; x++) {
                Vector3d dir = forward + right * ((x - width / 2.0) * pixpitch) +
                               down * ((y - height / 2.0) * pixpitch);
                dir.Normalize();

                // black where the ray exits the world
                unit.SetResult(x, y, Cast(Ray(camera, dir)).value_or(colorf(0)));
            }
        }

        m_Master.ReturnWorkUnit(unit);
    }

    std::cout << "end trace" << std::endl;
}
