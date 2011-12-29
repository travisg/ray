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
#ifndef __COLOR_H
#define __COLOR_H

#include <ostream>
#include <cstdlib>
#include <libvec/Random.h>

struct color32;
struct colorf;

struct color32 {
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;

	color32() {}
	color32(int c) : r(c), g(c), b(c) {}
	color32(const colorf &c);

	operator unsigned int() { return xrgb(); }

	unsigned int xrgb() { return ((r << 16) | (g << 8) | (b)); }
	unsigned int xbgr() { return ((b << 16) | (g << 8) | (r)); }
	unsigned int rgbx() { return ((r << 24) | (g << 16) | (b << 8)); }
	unsigned int bgrx() { return ((b << 24) | (g << 16) | (r << 8)); }
};

struct colorf {
public:
	double r;
	double g;
	double b;

	colorf() {}
	colorf(double _r, double _g, double _b) : r(_r), g(_g), b(_b) {}
	colorf(double c) : r(c), g(c), b(c) {}

	colorf operator*(double f);
	colorf operator*(const colorf &c);

	colorf & operator+=(const colorf &c);
	colorf & operator*=(double f);
	
	static colorf RandColor();
};

inline color32::color32(const colorf &c) 
{
	r = (c.r < 0.0f) ? 0 : ((c.r > 1.0f) ? 255 : ((unsigned char)(255.0 * c.r)));
	g = (c.g < 0.0f) ? 0 : ((c.g > 1.0f) ? 255 : ((unsigned char)(255.0 * c.g)));
	b = (c.b < 0.0f) ? 0 : ((c.b > 1.0f) ? 255 : ((unsigned char)(255.0 * c.b)));
}

inline colorf & colorf::operator+=(const colorf &c)
{
	r += c.r;
	g += c.g;
	b += c.b;

	return *this;
}

inline colorf & colorf::operator*=(double f)
{
	r *= f;
	g *= f;
	b *= f;

	return *this;
}

inline colorf colorf::operator*(double f)
{
	return colorf(r * f, g * f, b * f);
}

inline colorf colorf::operator*(const colorf &c)
{
	return colorf(r * c.r, g * c.g, b * c.b);
}

inline std::ostream &operator<<(std::ostream &os, const colorf &c)
{
	os << "< " << c.r << " " << c.g << " " << c.b << " >";
	return os;
}

inline colorf colorf::RandColor()
{
	return colorf(Libvec::Random(), Libvec::Random(), Libvec::Random());
}

#endif

