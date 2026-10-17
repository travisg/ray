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

#include <bounding_sphere.h>
#include <drawables/mesh_drawable.h>
#include <mesh/geometry.h>

using Libvec::Vector3d;

MeshDrawable::MeshDrawable(std::unique_ptr<Geometry> geom) : m_Geom(std::move(geom)) {}

MeshDrawable::~MeshDrawable() = default;

int MeshDrawable::Prepare() {
    // center the bounding sphere on the vertex average, radius to the farthest vertex
    if (m_Geom->Vertices().empty()) {
        return 0;
    }

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

namespace {

std::optional<Hit> TriIntersect(const Ray &ray, const Vector3d &v0, const Vector3d &v1,
                                const Vector3d &v2, const Vector3d &n0, const Vector3d &n1,
                                const Vector3d &n2) {
    Vector3d normal = Cross(v1 - v0, v2 - v0);

    // is the ray pointing at the surface?
    if (Dot(ray.dir, normal) > 0) {
        return std::nullopt;
    }

    // t = ((P - P0) . N) / (V . N)
    // P = P0 + tV
    double t = (Dot(v0 - ray.origin, normal)) / (Dot(ray.dir, normal));

    // is the origin behind the start of the ray
    if (t < 0.0) {
        return std::nullopt;
    }

    Vector3d pos = ray.origin + ray.dir * t;

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
    double b1 = (uv * wv - vv * wu) * D;
    if (b1 < 0.0 || b1 > 1.0) {
        return std::nullopt;
    }
    double b2 = (uv * wu - uu * wv) * D;
    if (b2 < 0.0 || (b1 + b2) > 1.0) {
        return std::nullopt;
    }

    Hit hit;
    hit.t = t;
    hit.pos = pos;
    // lerp the normal from the passed in normal vertexes
    hit.normal = n0 * (1 - (b1 + b2)) + n1 * b1 + n2 * b2;
    hit.normal.Normalize();
    return hit;
}

std::optional<Hit> QuadIntersect(const Ray &ray, const Vector3d &v0, const Vector3d &v1,
                                 const Vector3d &v2, const Vector3d &v3, const Vector3d &n0,
                                 const Vector3d &n1, const Vector3d &n2, const Vector3d &n3) {
    if (auto hit = TriIntersect(ray, v0, v1, v2, n0, n1, n2)) {
        return hit;
    }
    return TriIntersect(ray, v2, v3, v0, n2, n3, n0);
}

std::optional<Hit> SurfaceIntersect(const Ray &ray, const Mesh &mesh, const Surface &surface) {
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
        return TriIntersect(ray, v0, v1, v2, n0, n1, n2);
    }

    vp = &verts[surface.m_Indexes[3].posIndex];
    Vector3d v3(vp->x, vp->y, vp->z);
    vp = &nverts[surface.m_Indexes[3].normIndex];
    Vector3d n3(vp->x, vp->y, vp->z);

    return QuadIntersect(ray, v0, v1, v2, v3, n0, n1, n2, n3);
}

} // namespace

std::optional<Hit> MeshDrawable::Intersect(const Ray &ray) const {
    if (!m_BSphere.DoesIntersect(ray)) {
        return std::nullopt;
    }

    // nearest surface over every mesh
    std::optional<Hit> best;
    for (const auto &m : m_Geom->Meshes()) {
        for (const Surface &s : m->GetSurfaceList()) {
            std::optional<Hit> hit = SurfaceIntersect(ray, *m, s);
            if (hit && (!best || hit->t < best->t)) {
                best = hit;
            }
        }
    }

    return best;
}
