#ifndef __COLOR_H
#define __COLOR_H

#include <ostream>

struct color32;
struct colorf;

struct color32 {
public:
	union {		
		struct {
			unsigned char r;
			unsigned char g;
			unsigned char b;
		};
		unsigned int i;
	};

	color32() {}
	color32(int c) : r(c), g(c), b(c) {}
	color32(const colorf &c);

	operator unsigned int() { return ((r << 16) | (g << 8) | (b)); }
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

	colorf & operator+=(const colorf &c);
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

inline colorf colorf::operator*(double f)
{
	return colorf(r * f, g * f, b * f);
}

inline std::ostream &operator<<(std::ostream &os, const colorf &c)
{
	os << "< " << c.r << " " << c.g << " " << c.b << " >";
	return os;
}

#endif

