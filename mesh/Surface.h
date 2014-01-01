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
#ifndef __SURFACE_H
#define __SURFACE_H

#include <vector>

struct SurfaceIndex {
    int posIndex;
    int normIndex;
    int uvIndex;
};

typedef std::vector<SurfaceIndex> SurfaceIndexList;
typedef std::vector<SurfaceIndex>::iterator SurfaceIndexListIterator;
typedef std::vector<SurfaceIndex>::const_iterator SurfaceIndexListIteratorConst;

class Surface {
public:
    void AddIndex(int posIndex, int normIndex, int uvIndex);

    SurfaceIndexList m_Indexes;
};

typedef std::vector<Surface *> SurfaceList;
typedef std::vector<Surface *>::iterator SurfaceListIterator;
typedef std::vector<Surface *>::const_iterator SurfaceListIteratorConst;

#endif
