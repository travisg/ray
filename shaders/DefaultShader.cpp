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
#include "DefaultShader.h"
#include <Ray.h>
#include <Scene.h>
#include <Tracer.h>
#include <lights/SimpleLight.h>

using Libvec::Vector3d;

colorf DefaultShader::Run(const ShaderArgs &args) const {
    colorf color = args.scene->GetAmbientLight();

    for (const SimpleLight &l : args.scene->GetLightList()) {
        // cast a ray at each light, see if we're in a shadow
        Ray ray;
        ray.origin = args.pos + args.normal * 0.001f;
        ray.dir = l.GetPos() - ray.origin;
        ray.dir.Normalize();

        if (!args.scene->DoesIntersect(ray, (ray.origin - l.GetPos()).Length())) {
            Vector3d suntosurface = l.GetPos() - args.pos;
            suntosurface.Normalize();

            float light = Dot(suntosurface, args.normal);

            color += (l.GetColor() * light) * m_DiffuseColor;
        }
    }

    // see how much of it is a pure reflection
    if (m_Shinyness > 0.0f) {
        Ray ray;
        ray.origin = args.pos;

        // calculate the reflection ray
        float d = Dot(args.ray->dir, args.normal);

        Vector3d reflect(args.ray->dir.getx() - 2.0 * d * args.normal.getx(),
                         args.ray->dir.gety() - 2.0 * d * args.normal.gety(),
                         args.ray->dir.getz() - 2.0 * d * args.normal.getz());

        ray.dir = reflect;

        color *= (1.0f - m_Shinyness);

        // recursively trace to see
        colorf reflectcolor;
        if (args.tracer->Cast(reflectcolor, ray)) {
            color += reflectcolor * m_Shinyness;
        }
    }

    return color;
}
