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
#ifndef __GEOMETRY_H
#define __GEOMETRY_H

#include "Mesh.h"

class Geometry {
public:
	Geometry();
	~Geometry();

	void AddVert(const Vertex &v);
	void AddNormalVert(const Vertex &v);
	void AddTangentVert(const Vertex &v);
	void AddUVVert(const Vertex &v);
	void AddMesh(Mesh *m);

	Mesh *CreateMesh();

	MeshListIteratorConst ListIterator() const { return m_Meshes.begin(); }
	MeshListIteratorConst ListEnd() const { return m_Meshes.end(); }

	VertexListIteratorConst VertexListIterator() const { return m_Verts.begin(); }
	VertexListIteratorConst VertexListEnd() const { return m_Verts.end(); }

	VertexListIteratorConst NormalVertexListIterator() const { return m_NormalVerts.begin(); }
	VertexListIteratorConst NormalVertexListEnd() const { return m_NormalVerts.end(); }

	void Dump() const;

private:
	MeshList m_Meshes;

	// shared verts
	VertexList m_Verts;
	VertexList m_NormalVerts;
	VertexList m_TangentVerts;
	VertexList m_UVVerts;
};

#endif
