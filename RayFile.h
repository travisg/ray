#ifndef __RAYFILE_H
#define __RAYFILE_H

#define RAY_HEADER_MAGIC 'RAY1'

struct RayHeader {
	uint32_t magic;
	uint32_t width;
	uint32_t height;
};

enum RayDataType {
	TYPE_NULL = 0,
	TYPE_PIXEL,
	TYPE_PIXEL_RUN,
	TYPE_EOF,
};

struct RayDataPixel {
	uint32_t x;
	uint32_t y;
	float r, g, b;
};

struct RayDataPixelRun {
	uint32_t x;
	uint32_t y;
	uint32_t length;
	float val[];
};

#endif
