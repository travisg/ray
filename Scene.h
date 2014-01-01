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
#ifndef __SCENE_H
#define __SCENE_H

#include <vector>
#include "Color.h"
#include <libvec/Vector3.h>

class Drawable;
struct Ray;
class SimpleLight;

class Scene {
public:
    Scene();
    virtual ~Scene();

    const Drawable *Intersect(const Ray &ray);
    bool DoesIntersect(const Ray &ray);
    bool DoesIntersect(const Ray &ray, double maxDistance);

    const std::vector<SimpleLight *> GetLightList() const { return m_SimpleLightList; }
    colorf GetAmbientLight() const { return m_AmbientLight; }

    const Libvec::Vector3d &GetCamera() const { return m_Camera; }
    const Libvec::Vector3d &GetTarget() const { return m_Target; }

private:
    std::vector<Drawable *> m_DrawableList;;
    std::vector<SimpleLight *> m_SimpleLightList;
    colorf m_AmbientLight;

    Libvec::Vector3d m_Camera;
    Libvec::Vector3d m_Target;
};


#endif

