/*
 * Copyright (c) 2026 Travis Geiselbrecht
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
#include <ray/bvh.h>

#include <algorithm>
#include <cmath>

void bvh::build(std::vector<aabb> boxes) {
    nodes_.clear();
    prims_.clear();
    if (boxes.empty()) {
        return;
    }
    // pad every box a little so a surface lying exactly on a box face, as the quads of an
    // axis-aligned cube do, cannot slip through a slab test by one rounding error
    for (aabb &b : boxes) {
        double scale = 1.0;
        for (int a = 0; a < 3; a++) {
            scale = std::max({ scale, std::fabs(b.min[a]), std::fabs(b.max[a]) });
        }
        const double pad = 1e-9 * scale;
        b.min -= libvec::vector3d(pad);
        b.max += libvec::vector3d(pad);
    }
    prims_.resize(boxes.size());
    for (uint32_t i = 0; i < prims_.size(); i++) {
        prims_[i] = i;
    }
    nodes_.reserve(2 * boxes.size());
    build_node(0, static_cast<uint32_t>(boxes.size()), boxes, 0);
}

uint32_t bvh::build_node(uint32_t begin, uint32_t end, const std::vector<aabb> &boxes, int depth) {
    const auto index = static_cast<uint32_t>(nodes_.size());
    nodes_.emplace_back();

    aabb box;
    aabb centroids;
    for (uint32_t i = begin; i < end; i++) {
        box.expand(boxes[prims_[i]]);
        centroids.expand(boxes[prims_[i]].centroid());
    }
    nodes_[index].box = box;

    const uint32_t count = end - begin;
    const int axis = centroids.longest_axis();
    // a leaf: few enough primitives, nothing left to split by, or out of stack
    if (count <= kLeafSize || centroids.extent()[axis] == 0 || depth + 2 >= kMaxDepth) {
        nodes_[index].first = begin;
        nodes_[index].count = count;
        return index;
    }

    const uint32_t mid = begin + count / 2;
    std::nth_element(prims_.begin() + begin, prims_.begin() + mid, prims_.begin() + end,
                     [&](uint32_t a, uint32_t b) {
                         return boxes[a].centroid()[axis] < boxes[b].centroid()[axis];
                     });
    build_node(begin, mid, boxes, depth + 1); // the left child is the next node
    const uint32_t right = build_node(mid, end, boxes, depth + 1);
    nodes_[index].right = right;
    return index;
}
