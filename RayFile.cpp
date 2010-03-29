#include <iostream>
#include "RayFile.h"


RayFile::RayFile()
:	m_fp(NULL),
	m_write(false),
	m_width(0),
	m_height(0)
{
}

RayFile::~RayFile()
{
	Close();
}

int RayFile::Open(const std::string &name)
{
	if (m_fp)
		Close();

	m_fp = fopen(name.c_str(), "rb");
	if (!m_fp)
		return -1;

	m_write = false;	

	RayHeader header;

	if (fread(&header, sizeof(header), 1, m_fp) < 1) {
		Close();
		return -1;
	}

	if (header.magic != RAY_HEADER_MAGIC) {
		Close();
		return -1;
	}

	m_width = header.width;
	m_height = header.height;

	return 0;
}

int RayFile::OpenWrite(const std::string &name, uint32_t width, uint32_t height)
{
	if (m_fp)
		Close();

	m_fp = fopen(name.c_str(), "wb+");
	if (!m_fp)
		return -1;

	m_write = true;

	RayHeader header;

	m_width = width;
	m_height = height;

	header.magic = RAY_HEADER_MAGIC;
	header.width = m_width;
	header.height = m_height;

	fwrite(&header, sizeof(header), 1, m_fp);

	return 0;
}

void RayFile::Close()
{
	if (m_fp) {
		if (m_write) {
			// write EOF token
			uint32_t type = TYPE_EOF;
			fwrite(&type, sizeof(type), 1, m_fp);
		}

		fclose(m_fp);
		m_fp = NULL;
		m_write = false;
	}
}

void RayFile::SetXY(int x, int y, colorf color)
{
	// write a pixel packet
	if (m_fp && m_write) {
		uint32_t type = TYPE_PIXEL;

		fwrite(&type, sizeof(type), 1, m_fp);

		RayDataPixel pixel;
		pixel.x = x;
		pixel.y = y;
		pixel.r = color.r;
		pixel.g = color.g;
		pixel.b = color.b;
		fwrite(&pixel, sizeof(pixel), 1, m_fp);
	}
}

void RayFile::SetXYRun(int x, int y, int count, const colorf *color)
{
//	std::cout << __PRETTY_FUNCTION__ << " " << x << " " << y << " " << count << std::endl;
	if (m_fp && m_write) {
		uint32_t type = TYPE_PIXEL_RUN;

		fwrite(&type, sizeof(type), 1, m_fp);

		RayDataPixelRun run;

		run.x = x;
		run.y = y;
		run.length = count;
		fwrite(&run, sizeof(run), 1, m_fp);

		float colors[3];

		for (int i = 0; i < count; i++) {
			colors[0] = color[i].r;
			colors[1] = color[i].g;
			colors[2] = color[i].b;
			fwrite(colors, sizeof(colors), 1, m_fp);
		}
	}
}

static int ReadData(FILE *fp, void *_buf, size_t len)
{
	if (fread(_buf, len, 1, fp) < 1)
		return -1;

	return len;
}

struct BufferReadStruct {
	float *buf;
	int width, height;
};

static int BufferCallback(void *buf, int x, int y, const float rgb[3])
{
	BufferReadStruct *s = (BufferReadStruct *)buf;

//	std::cout << "buf " << buf << " " << x << " " << y << std::endl;

	s->buf[(y * s->width + x) * 3] = rgb[0];
	s->buf[(y * s->width + x) * 3 + 1] = rgb[1];
	s->buf[(y * s->width + x) * 3 + 2] = rgb[2];

	return 0;
}

int RayFile::ReadIntoBuffer(float *buf)
{
	BufferReadStruct s;

	s.buf = buf;
	s.width = m_width;
	s.height = m_height;

	return Parse(&BufferCallback, &s);
}

int RayFile::Parse(int (*callback)(void *, int x, int y, const float rgb[3]), void *arg)
{
	if (!m_fp || m_write)
		return -1;

	for (;;) {
		uint32_t type;
		
		if (ReadData(m_fp, &type, sizeof(type)) < 0)
			goto done;
//		printf("type %d\n", type);

		switch (type) {
			case TYPE_PIXEL: {
				RayDataPixel pixel;

				if (ReadData(m_fp, &pixel, sizeof(pixel)) < 0)
					goto done;

//				printf("pixel data: x %d y %d, r %f g %f b %f\n", pixel.x, pixel.y, pixel.r, pixel.g, pixel.b);

				float rgb[3] = { pixel.r, pixel.g, pixel.b };
				callback(arg, pixel.x, pixel.y, rgb);
				break;
			}
			case TYPE_PIXEL_RUN: {
				RayDataPixelRun run;

				if (ReadData(m_fp, &run, sizeof(run)) < 0)
					goto done;

//				printf("pixel run data: x %d y %d, len %d\n", run.x, run.y, run.length);

				for (unsigned int i = 0; i < run.length; i++) {
					float c[3];
					if (ReadData(m_fp, &c, sizeof(c)) < 0)
						goto done;

					callback(arg, run.x + i, run.y, c);
				}		
				break;
			}
			case TYPE_EOF:
				goto done;
			case TYPE_NULL:
			default:
				;
		}

	}
done:
	printf("hit EOF\n");

	return 0;

}


