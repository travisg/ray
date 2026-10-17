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

#include <Ray.h>
#include <Scene.h>
#include <Tracer.h>
#include <drawables/Drawable.h>
#include <shaders/Shader.h>

using Libvec::Vector3d;

Tracer::Tracer(const RenderSurface &surface, const Scene &scene, TraceMaster &tracemaster)
    : m_Surface(surface), m_Scene(scene), m_Master(tracemaster) {}

bool Tracer::Cast(colorf &color, const Ray &ray) {
    if (m_Depth == kMaxTraceDepth) {
        return false;
    }

    // find the closest object along the ray
    const Drawable *d = m_Scene.Intersect(ray);
    if (!d) {
        return false;
    }

    Vector3d pos;
    Vector3d normal;
    d->Intersect(ray, pos, normal);

    const ShaderPtr &s = d->GetShader();
    if (!s) {
        color = 0;
    } else {
        ShaderArgs args;
        args.scene = &m_Scene;
        args.tracer = this;
        args.d = d;
        args.ray = &ray;
        args.pos = pos;
        args.normal = normal;

        m_Depth++; // track the level of recursion
        color = s->Run(args);
        m_Depth--;
    }

    return true;
}

void Tracer::Trace() {
    int width = m_Surface.Width();
    int height = m_Surface.Height();

    Libvec::Vector3d target = m_Scene.GetTarget();
    target -= m_Scene.GetCamera();
    target.Normalize();
    target += m_Scene.GetCamera();

    Vector3d lin = target - m_Scene.GetCamera();

    float pixpitch = 2.0 / width;

    TraceWorkUnit unit;
    while (m_Master.GetWorkUnit(unit) >= 0) {
        for (int y = unit.starty; y <= unit.endy; y++) {
            for (int x = unit.startx; x <= unit.endx; x++) {
                Vector3d wing(lin.gety(), -lin.getx(), 0);
                Vector3d head = Cross(lin, wing);

                wing.Normalize((x - width / 2.0) * pixpitch);
                head.Normalize((y - height / 2.0) * pixpitch);

                Libvec::Vector3d ray = target + wing + head - m_Scene.GetCamera();
                ray.Normalize();

                colorf color;
                if (!Cast(color, Ray(m_Scene.GetCamera(), ray))) {
                    // exited the world
                    color = 0;
                }
                unit.SetResult(x, y, color);
            }
        }

        m_Master.ReturnWorkUnit(unit);
    }

    std::cout << "end trace" << std::endl;
}
