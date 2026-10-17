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
#pragma once

#include <cstddef>
#include <cstdint>
#include <libvec/vector3.h>
#include <ray/aabb.h>
#include <ray/ray.h>
#include <vector>

// Bounding volume hierarchy over primitives the tree knows only by their boxes; the caller
// intersects the primitives a leaf names. Built by splitting at the median along the longest
// axis of the centroids, with leaves of up to kLeafSize primitives. Nodes are in depth-first
// order, so a node's left child follows it and only the right child's index is stored.
class bvh {
  public:
    static constexpr uint32_t kLeafSize = 4;

    // boxes[i] bounds primitive i; an empty list builds a tree nothing hits
    void build(std::vector<aabb> boxes);

    bool empty() const { return nodes_.empty(); }
    size_t node_count() const { return nodes_.size(); }
    const aabb &bounds() const { return nodes_.front().box; }

    // Calls visit(prim, t_max) for every primitive whose box the ray enters before t_max,
    // nearer boxes first. visit lowers t_max when it finds a hit, which prunes what follows;
    // boxes entered exactly at t_max are still visited so the caller can break ties.
    template <typename F> void intersect(const ray &r, F &&visit) const {
        if (nodes_.empty()) {
            return;
        }
        const libvec::vector3d inv_dir(1.0 / r.dir.x, 1.0 / r.dir.y, 1.0 / r.dir.z);
        double t_max = std::numeric_limits<double>::infinity();

        struct entry {
            uint32_t node;
            double t;
        };
        entry stack[kMaxDepth];
        int top = 0;
        if (std::optional<double> t = nodes_[0].box.intersect(r, inv_dir, t_max)) {
            stack[top++] = { 0, *t };
        }
        while (top > 0) {
            const entry e = stack[--top];
            if (e.t > t_max) {
                continue; // a nearer hit arrived after this was pushed
            }
            const node &n = nodes_[e.node];
            if (n.count > 0) {
                for (uint32_t i = 0; i < n.count; i++) {
                    visit(prims_[n.first + i], t_max);
                }
                continue;
            }
            const uint32_t left = e.node + 1;
            const uint32_t right = n.right;
            std::optional<double> tl = nodes_[left].box.intersect(r, inv_dir, t_max);
            std::optional<double> tr = nodes_[right].box.intersect(r, inv_dir, t_max);
            // push the farther child first so the nearer one is popped next
            if (tl && tr && *tr < *tl) {
                stack[top++] = { left, *tl };
                stack[top++] = { right, *tr };
            } else {
                if (tr) {
                    stack[top++] = { right, *tr };
                }
                if (tl) {
                    stack[top++] = { left, *tl };
                }
            }
        }
    }

  private:
    // deep enough for a median split of 2^32 primitives, with room to spare
    static constexpr int kMaxDepth = 64;

    struct node {
        aabb box;
        uint32_t first = 0; // leaf: index into prims_
        uint32_t count = 0; // leaf: primitives; 0 marks an interior node
        uint32_t right = 0; // interior: right child
    };

    uint32_t build_node(uint32_t begin, uint32_t end, const std::vector<aabb> &boxes, int depth);

    std::vector<node> nodes_;
    std::vector<uint32_t> prims_; // primitive indices, grouped by leaf
};
