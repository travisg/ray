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
#include <algorithm>
#include <lights/SimpleLight.h>
#include <optional>

using Libvec::Vector3d;

namespace {

// Secondary rays start this far above the surface along its normal.
constexpr double kBias = 0.001;

} // namespace

colorf DefaultShader::Run(const ShaderArgs &args) const {
    const Hit &hit = args.hit;
    const Vector3d origin = hit.pos + hit.normal * kBias;
    colorf color = args.scene.GetAmbientLight();

    for (const SimpleLight &l : args.scene.GetLightList()) {
        // shadow ray toward the light
        Vector3d tolight = l.GetPos() - origin;
        const double distance = tolight.Length();
        tolight.Normalize();
        if (args.scene.DoesIntersect(Ray(origin, tolight), distance)) {
            continue;
        }

        const double diffuse = std::max(0.0, Dot(tolight, hit.normal));
        color += (l.GetColor() * diffuse) * m_DiffuseColor;
    }

    if (m_Shinyness > 0.0) {
        // mirror reflection
        const double d = Dot(args.ray.dir, hit.normal);
        const Ray reflected(origin, args.ray.dir - hit.normal * (2.0 * d));

        color *= (1.0 - m_Shinyness);
        if (std::optional<colorf> reflection = args.tracer.Cast(reflected)) {
            color += *reflection * m_Shinyness;
        }
    }

    return color;
}
