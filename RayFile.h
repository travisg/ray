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
    uint32_t Width() const { return m_width; }
    uint32_t Height() const { return m_height; }

    /* write routines */
    void SetXY(int x, int y, colorf color);
    void SetXYRun(int x, int y, int count, const colorf *color);
    void SetXYRun(int x, int y, int count, const float *color);

    /* read routines */
    int ReadIntoBuffer(float *buf); // up to the caller to make sure it's big enough

    // return a pointer to a mmapped file

    /* read the file in, parsing it, calling back on every pixel */
    int Parse(int (*callback)(RayFile &, void *, int x, int y, const float rgb[3]), void *arg);


private:
    FILE *m_fp;
    bool m_write;
    uint32_t m_width, m_height;
};

/* support routines */

int ReadIntoMmap(RayFile &file, const std::string &mmapfile, float **buf, uint64_t *len);

/* TGA */
int ConvertToTGA(RayFile &file, const std::string &filename);

#endif
