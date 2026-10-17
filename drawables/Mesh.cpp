/*
 * Copyright (c) 2008-2012 Travis Geiselbrecht
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
#include <iostream>
#include <memory>
#include <vector>

#include <BoundingSphere.h>
#include <drawables/Mesh.h>
#include <mesh/Geometry.h>

using Libvec::Vector3d;

MeshDrawable::MeshDrawable(std::shared_ptr<Geometry> g) : m_Geom(std::move(g)) {}

int MeshDrawable::Prepare() {
    // center the bounding sphere on the vertex average, radius to the farthest vertex
    Vector3d avg(0, 0, 0);
    int count = 0;
    for (const Vertex &v : m_Geom->Vertices()) {
        avg += Vector3d(v.x, v.y, v.z);
        count++;
    }
    avg /= count;

    m_BSphere.center = avg;

    double farthest = 0;
    for (const Vertex &vert : m_Geom->Vertices()) {
        Vector3d v(vert.x, vert.y, vert.z);

        double dist = (v - m_BSphere.center).Length();
        if (dist > farthest) {
            farthest = dist;
        }
    }

    m_BSphere.radius = farthest;

    std::cout << "mesh bounding sphere center " << avg << " radius " << farthest << std::endl;

    return 0;
}

bool MeshDrawable::Intersect(const Ray &ray) const {
    if (!m_BSphere.DoesIntersect(ray)) {
        return false;
    }

    return false;
}

namespace {

bool TriIntersect(const Ray &ray, Vector3d &pos, Vector3d &normal, const Vector3d &v0,
                  const Vector3d &v1, const Vector3d &v2, const Vector3d &n0, const Vector3d &n1,
                  const Vector3d &n2) {
    normal = Cross(v1 - v0, v2 - v0);

    // is the ray pointing at the surface?
    if (Dot(ray.dir, normal) > 0) {
        return false;
    }

    // t = ((P - P0) . N) / (V . N)
    // P = P0 + tV
    double t = (Dot(v0 - ray.origin, normal)) / (Dot(ray.dir, normal));

    // is the origin behind the start of the ray
    if (t < 0.0) {
        return false;
    }

    pos = ray.origin + ray.dir * t;

    // it intersects the plane the triangle is in, now calc the u/v coordinates and see if it's in
    // the triangle
    double uu, uv, vv, wu, wv, D;

    Vector3d u = v1 - v0;
    Vector3d v = v2 - v0;

    uu = Dot(u, u);
    uv = Dot(u, v);
    vv = Dot(v, v);
    Vector3d w = pos - v0;
    wu = Dot(w, u);
    wv = Dot(w, v);
    D = 1.0 / (uv * uv - uu * vv);

    // get and test parametric coords
    double s = (uv * wv - vv * wu) * D;
    if (s < 0.0 || s > 1.0) { // I is outside T
        return false;
    }
    t = (uv * wu - uu * wv) * D;
    if (t < 0.0 || (s + t) > 1.0) { // I is outside T
        return false;
    }

    // lerp the normal from the passed in normal vertexes
    normal = n0 * (1 - (s + t)) + n1 * s + n2 * t;
    normal.Normalize();

    return true;
}

bool QuadIntersect(const Ray &ray, Vector3d &pos, Vector3d &normal, const Vector3d &v0,
                   const Vector3d &v1, const Vector3d &v2, const Vector3d &v3, const Vector3d &n0,
                   const Vector3d &n1, const Vector3d &n2, const Vector3d &n3) {
    if (TriIntersect(ray, pos, normal, v0, v1, v2, n0, n1, n2)) {
        return true;
    }
    return TriIntersect(ray, pos, normal, v2, v3, v0, n2, n3, n0);
}

bool SurfaceIntersect(const Ray &ray, Vector3d &pos, Vector3d &normal, const Mesh &mesh,
                      const Surface &surface) {
    // get the vertexes that make up this surface
    const VertexList &verts = mesh.GetVertexList();

    const Vertex *vp = &verts[surface.m_Indexes[0].posIndex];
    Vector3d v0(vp->x, vp->y, vp->z);
    vp = &verts[surface.m_Indexes[1].posIndex];
    Vector3d v1(vp->x, vp->y, vp->z);
    vp = &verts[surface.m_Indexes[2].posIndex];
    Vector3d v2(vp->x, vp->y, vp->z);

    // get the normal vertexes
    const VertexList &nverts = mesh.GetNormalVertexList();
    vp = &nverts[surface.m_Indexes[0].normIndex];
    Vector3d n0(vp->x, vp->y, vp->z);
    vp = &nverts[surface.m_Indexes[1].normIndex];
    Vector3d n1(vp->x, vp->y, vp->z);
    vp = &nverts[surface.m_Indexes[2].normIndex];
    Vector3d n2(vp->x, vp->y, vp->z);

    if (surface.m_Indexes.size() == 3) {
        return TriIntersect(ray, pos, normal, v0, v1, v2, n0, n1, n2);
    }

    vp = &verts[surface.m_Indexes[3].posIndex];
    Vector3d v3(vp->x, vp->y, vp->z);
    vp = &nverts[surface.m_Indexes[3].normIndex];
    Vector3d n3(vp->x, vp->y, vp->z);

    return QuadIntersect(ray, pos, normal, v0, v1, v2, v3, n0, n1, n2, n3);
}

} // namespace

bool MeshDrawable::Intersect(const Ray &ray, Vector3d &pos, Vector3d &normal) const {
    if (!m_BSphere.DoesIntersect(ray)) {
        return false;
    }

    const Mesh *meshmatch = nullptr;
    const Surface *surfmatch = nullptr;
    double matchdist = -1;

    // for every mesh, check every surface
    for (const Mesh *m : m_Geom->Meshes()) {
        for (const Surface *s : m->GetSurfaceList()) {
            if (!SurfaceIntersect(ray, pos, normal, *m, *s)) {
                continue;
            }

            // surface intersected, lets see if it's the best match
            double distance = (pos - ray.origin).LengthSquared();
            if (matchdist < 0 || distance < matchdist) {
                meshmatch = m;
                surfmatch = s;
                matchdist = distance;
            }
        }
    }

    // rerun the closest match to the caster
    if (meshmatch) {
        SurfaceIntersect(ray, pos, normal, *meshmatch, *surfmatch);
        return true;
    }

    return false;
}
