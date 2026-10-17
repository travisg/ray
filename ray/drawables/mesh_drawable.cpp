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
#include <memory>
#include <vector>

#include <ray/aabb.h>
#include <ray/drawables/mesh_drawable.h>
#include <ray/mesh/geometry.h>

using libvec::vector3d;

mesh_drawable::mesh_drawable(std::unique_ptr<geometry> geom) : geometry_(std::move(geom)) {}

mesh_drawable::~mesh_drawable() = default;

int mesh_drawable::prepare() {
    // one box per face, numbered in mesh then surface order
    faces_.clear();
    std::vector<aabb> boxes;
    const auto &meshes = geometry_->meshes();
    for (uint32_t mi = 0; mi < meshes.size(); mi++) {
        const vertex_list &verts = meshes[mi]->vertices();
        const auto &surfaces = meshes[mi]->surfaces();
        for (uint32_t si = 0; si < surfaces.size(); si++) {
            aabb box;
            for (const surface_index &idx : surfaces[si].indexes) {
                const vertex &v = verts[idx.pos_index];
                box.expand(vector3d(v.x, v.y, v.z));
            }
            boxes.push_back(box);
            faces_.push_back({ mi, si });
        }
    }
    bvh_.build(std::move(boxes));
    return 0;
}

namespace {

std::optional<hit> tri_intersect(const ray &r, const vector3d &v0, const vector3d &v1,
                                 const vector3d &v2, const vector3d &n0, const vector3d &n1,
                                 const vector3d &n2) {
    vector3d normal = cross(v1 - v0, v2 - v0);

    // is the ray pointing at the surface?
    if (dot(r.dir, normal) > 0) {
        return std::nullopt;
    }

    // t = ((P - P0) . N) / (V . N)
    // P = P0 + tV
    double t = (dot(v0 - r.origin, normal)) / (dot(r.dir, normal));

    // is the origin behind the start of the ray
    if (t < 0.0) {
        return std::nullopt;
    }

    vector3d pos = r.origin + r.dir * t;

    // it intersects the plane the triangle is in, now calc the u/v coordinates and see if it's in
    // the triangle
    double uu, uv, vv, wu, wv, d;

    vector3d u = v1 - v0;
    vector3d v = v2 - v0;

    uu = dot(u, u);
    uv = dot(u, v);
    vv = dot(v, v);
    vector3d w = pos - v0;
    wu = dot(w, u);
    wv = dot(w, v);
    d = 1.0 / (uv * uv - uu * vv);

    // get and test parametric coords
    double b1 = (uv * wv - vv * wu) * d;
    if (b1 < 0.0 || b1 > 1.0) {
        return std::nullopt;
    }
    double b2 = (uv * wu - uu * wv) * d;
    if (b2 < 0.0 || (b1 + b2) > 1.0) {
        return std::nullopt;
    }

    hit h;
    h.t = t;
    h.pos = pos;
    // lerp the normal from the passed in normal vertexes
    h.normal = n0 * (1 - (b1 + b2)) + n1 * b1 + n2 * b2;
    h.normal.normalize();
    return h;
}

std::optional<hit> quad_intersect(const ray &r, const vector3d &v0, const vector3d &v1,
                                  const vector3d &v2, const vector3d &v3, const vector3d &n0,
                                  const vector3d &n1, const vector3d &n2, const vector3d &n3) {
    if (auto h = tri_intersect(r, v0, v1, v2, n0, n1, n2)) {
        return h;
    }
    return tri_intersect(r, v2, v3, v0, n2, n3, n0);
}

std::optional<hit> surface_intersect(const ray &r, const mesh &m, const surface &sf) {
    // get the vertexes that make up this surface
    const vertex_list &verts = m.vertices();

    const vertex *vp = &verts[sf.indexes[0].pos_index];
    vector3d v0(vp->x, vp->y, vp->z);
    vp = &verts[sf.indexes[1].pos_index];
    vector3d v1(vp->x, vp->y, vp->z);
    vp = &verts[sf.indexes[2].pos_index];
    vector3d v2(vp->x, vp->y, vp->z);

    // get the normal vertexes
    const vertex_list &nverts = m.normals();
    vp = &nverts[sf.indexes[0].norm_index];
    vector3d n0(vp->x, vp->y, vp->z);
    vp = &nverts[sf.indexes[1].norm_index];
    vector3d n1(vp->x, vp->y, vp->z);
    vp = &nverts[sf.indexes[2].norm_index];
    vector3d n2(vp->x, vp->y, vp->z);

    if (sf.indexes.size() == 3) {
        return tri_intersect(r, v0, v1, v2, n0, n1, n2);
    }

    vp = &verts[sf.indexes[3].pos_index];
    vector3d v3(vp->x, vp->y, vp->z);
    vp = &nverts[sf.indexes[3].norm_index];
    vector3d n3(vp->x, vp->y, vp->z);

    return quad_intersect(r, v0, v1, v2, v3, n0, n1, n2, n3);
}

} // namespace

std::optional<hit> mesh_drawable::intersect(const ray &r) const {
    // nearest face; equal distances go to the lower face number, as a plain scan would
    std::optional<hit> best;
    uint32_t best_face = 0;
    const auto &meshes = geometry_->meshes();
    bvh_.intersect(r, [&](uint32_t fi, double &t_max) {
        const face &f = faces_[fi];
        const mesh &m = *meshes[f.mesh];
        std::optional<hit> h = surface_intersect(r, m, m.surfaces()[f.surface]);
        if (h && (!best || h->t < best->t || (h->t == best->t && fi < best_face))) {
            best = h;
            best_face = fi;
            t_max = h->t;
        }
    });
    return best;
}
