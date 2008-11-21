#ifndef __COLOR_H
#define __COLOR_H

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

	operator unsigned int() { return ((r << 16) | (g << 8) | (b)); }
};

struct colorf {
public:
	float r;
	float g;
	float b;

	colorf() {}
	colorf(float c) : r(c), g(c), b(c) {}
};

inline color32::color32(const colorf &c) 
{
	r = (c.r < 0.0f) ? 0 : ((c.r > 1.0f) ? 255 : ((unsigned char)(255.0 * c.r)));
	g = (c.g < 0.0f) ? 0 : ((c.g > 1.0f) ? 255 : ((unsigned char)(255.0 * c.g)));
	b = (c.b < 0.0f) ? 0 : ((c.b > 1.0f) ? 255 : ((unsigned char)(255.0 * c.b)));
}

#endif

