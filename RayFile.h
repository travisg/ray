#ifndef __RAYFILE_H
#define __RAYFILE_H

#include <inttypes.h>
#include <stdio.h>
#include <string>
#include "Color.h"

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

class RayFile {
public:
	RayFile();
	~RayFile();

	int Open(const std::string &name);
	int OpenWrite(const std::string &name, uint32_t width, uint32_t height);
	void Close();

	/* query */
	uint64_t GetPixelCount() const { return m_width * m_height; }

	/* write routines */
	void SetXY(int x, int y, colorf color);
	void SetXYRun(int x, int y, int count, const colorf *color);

	/* read routines */
	int ReadIntoBuffer(float *buf); // up to the caller to make sure it's big enough

	/* read the file in, parsing it, calling back on every color */
	int Parse(int (*callback)(void *, int x, int y, const float rgb[3]), void *arg);

private:
	FILE *m_fp;
	bool m_write;
	uint32_t m_width, m_height;
};

#endif
