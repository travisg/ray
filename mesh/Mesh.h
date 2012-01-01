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
#ifndef __MESH_H
#define __MESH_H

#include "Vertex.h"
#include "Surface.h"
#include <string>

class Mesh {
public:
	Mesh(const VertexList &posVerts, const VertexList &normalVerts, const VertexList &tangentVerts, const VertexList &uvVerts);
	virtual ~Mesh();

	void AddSurface(Surface *s);

	void SetName(const std::string &name);
	std::string GetName() { return m_Name; }

	void SetVertexLists();

	void Dump() const;

private:
	std::string m_Name;

	// vertex list, probably shared between multiple meshes
	const VertexList &m_Verts;
	const VertexList &m_NormalVerts;
	const VertexList &m_TangentVerts;
	const VertexList &m_UVVerts;

	SurfaceList m_Surfaces;

	friend class UnifiedMesh;
};

typedef std::vector<Mesh*> MeshList;
typedef std::vector<Mesh*>::iterator MeshListIterator;
typedef std::vector<Mesh*>::const_iterator MeshListIteratorConst;

#endif
