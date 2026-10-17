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

#include "color.h"
#include "shader.h"
#include <algorithm>

// Ambient + diffuse point lights with shadow rays, plus a mirror reflection
// weighted by shininess.
class DefaultShader : public Shader {
  public:
    colorf Run(const ShaderArgs &args) const override;

    // 0 = matte, 1 = perfect mirror
    void SetShinyness(double shiny) { m_Shinyness = std::clamp(shiny, 0.0, 1.0); }
    void SetDiffuseColor(colorf diffuse) { m_DiffuseColor = diffuse; }

  private:
    double m_Shinyness = 0.0;
    colorf m_DiffuseColor = colorf(1.0, 1.0, 1.0);
};
