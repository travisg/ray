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
#ifndef __DEFAULT_SHADER_H
#define __DEFAULT_SHADER_H

#include "Shader.h"
#include "Color.h"

class DefaultShader : public Shader {
public:
	DefaultShader();
	virtual ~DefaultShader();

	virtual colorf Run(const ShaderArgs &args);

	void SetShinyness(float shiny) { if (shiny > 1.0f) shiny = 1.0f; if (shiny < 0) shiny = 0; m_Shinyness = shiny; }
	void SetDiffuseColor(colorf diffuse) { m_DiffuseColor = diffuse; }

private:
	float   m_Shinyness;
	colorf	m_DiffuseColor;
};

#endif

