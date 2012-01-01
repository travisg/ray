/*
 * Copyright (c) 2008-2012 Travis Geiselbrecht
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

#include <Scene.h>
#include <Ray.h>
#include <Tracer.h>
#include <lights/SimpleLight.h>
#include "DebugShader.h"

using Libvec::Vector3d;

DebugShader::DebugShader()
:	mode(MODE_NORMAL)
{
}

DebugShader::~DebugShader()
{
}

colorf DebugShader::Run(const ShaderArgs &args)
{
//	std::cout << "DebugShader Run" << std::endl;

	colorf color;
	switch (mode) {
		case MODE_NORMAL:
			color = colorf(args.normal.getx(), args.normal.gety(), args.normal.getz());
			break;
		case MODE_DEPTH:
			color = (args.ray->origin - args.pos).Length();
			break;
		default:
			color = 0;
	}


//	std::cout << "debug shader color " << color << std::endl;
	return color;
}

