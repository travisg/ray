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
#pragma once

#include <libvec/Vector3.h>
#include <shaders/Shader.h>
#include <utility>

struct Ray;

class Drawable {
  protected:
    Drawable() = default;

  public:
    virtual ~Drawable() = default;
    Drawable(const Drawable &) = delete;
    Drawable &operator=(const Drawable &) = delete;

    virtual int Prepare() { return 0; }

    virtual bool Intersect(const Ray &ray) const = 0;
    virtual bool Intersect(const Ray &ray, Libvec::Vector3d &pos,
                           Libvec::Vector3d &normal) const = 0;

    void SetShader(ShaderPtr s) { m_Shader = std::move(s); }
    const ShaderPtr &GetShader() const { return m_Shader; }

  private:
    ShaderPtr m_Shader;
};
