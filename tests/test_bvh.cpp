#include <cstdint>
#include <optional>
#include <random>
#include <ray/aabb.h>
#include <ray/bvh.h>
#include <ray/ray.h>
#include <tests/check.h>
#include <vector>

using libvec::vector3d;

namespace {

// The primitives are boxes themselves: a hit is where the ray enters the primitive's box.
struct nearest {
    std::optional<double> t;
    uint32_t prim = UINT32_MAX;
};

nearest brute_force(const std::vector<aabb> &boxes, const ray &r) {
    const vector3d inv(1.0 / r.dir.x, 1.0 / r.dir.y, 1.0 / r.dir.z);
    nearest best;
    for (uint32_t i = 0; i < boxes.size(); i++) {
        std::optional<double> t = boxes[i].intersect(r, inv, 1e300);
        if (t && (!best.t || *t < *best.t)) {
            best = { t, i };
        }
    }
    return best;
}

nearest with_tree(const bvh &tree, const std::vector<aabb> &boxes, const ray &r) {
    const vector3d inv(1.0 / r.dir.x, 1.0 / r.dir.y, 1.0 / r.dir.z);
    nearest best;
    tree.intersect(r, [&](uint32_t prim, double &t_max) {
        std::optional<double> t = boxes[prim].intersect(r, inv, 1e300);
        if (t && (!best.t || *t < *best.t || (*t == *best.t && prim < best.prim))) {
            best = { t, prim };
            t_max = *t;
        }
    });
    return best;
}

} // namespace

int main() {
    std::mt19937 rng(7);
    std::uniform_real_distribution<double> pos(0.0, 10.0);
    std::uniform_real_distribution<double> dir(-1.0, 1.0);

    // the slab test: hits, misses, starting inside, and axis-aligned rays
    {
        aabb b;
        b.expand(vector3d(1, 1, 1));
        b.expand(vector3d(2, 2, 2));
        const vector3d inv(1.0 / 1e-300, 1.0 / 1e-300, 1.0);
        CHECK(b.intersect(ray(vector3d(1.5, 1.5, 0), vector3d(0, 0, 1)),
                          vector3d(1.0 / 0.0, 1.0 / 0.0, 1.0), 10)
                  .value_or(-1) == 1.0);
        CHECK(!b.intersect(ray(vector3d(3, 1.5, 0), vector3d(0, 0, 1)),
                           vector3d(1.0 / 0.0, 1.0 / 0.0, 1.0), 10));
        CHECK(b.intersect(ray(vector3d(1.5, 1.5, 1.5), vector3d(0, 0, 1)),
                          vector3d(1.0 / 0.0, 1.0 / 0.0, 1.0), 10)
                  .value_or(-1) == 0.0);
        CHECK(!b.intersect(ray(vector3d(1.5, 1.5, 0), vector3d(0, 0, 1)),
                           vector3d(1.0 / 0.0, 1.0 / 0.0, 1.0), 0.5)); // enters beyond t_max
        CHECK(!b.intersect(ray(vector3d(1.5, 1.5, 5), vector3d(0, 0, 1)),
                           vector3d(1.0 / 0.0, 1.0 / 0.0, 1.0), 10)); // behind the origin
        (void)inv;
    }

    // an empty tree visits nothing
    {
        bvh tree;
        tree.build({});
        CHECK(tree.empty());
        int visits = 0;
        tree.intersect(ray(vector3d(0, 0, 0), vector3d(0, 0, 1)),
                       [&](uint32_t, double &) { visits++; });
        CHECK(visits == 0);
    }

    // random boxes: the tree finds exactly what brute force finds, ties going to the lower index
    {
        std::vector<aabb> boxes;
        for (int i = 0; i < 500; i++) {
            aabb b;
            const vector3d c(pos(rng), pos(rng), pos(rng));
            b.expand(c);
            b.expand(c + vector3d(pos(rng) * 0.3, pos(rng) * 0.3, pos(rng) * 0.3));
            boxes.push_back(b);
        }
        // a few exact duplicates, so ties happen
        for (int i = 0; i < 20; i++) {
            boxes.push_back(boxes[i * 7]);
        }
        bvh tree;
        tree.build(boxes);
        CHECK(!tree.empty());
        CHECK(tree.node_count() <= 2 * boxes.size());
        CHECK(tree.bounds().min.x <= 0.1 && tree.bounds().max.x >= 9.0);

        int hits = 0;
        for (int i = 0; i < 3000; i++) {
            vector3d d(dir(rng), dir(rng), dir(rng));
            if (i % 5 == 0) {
                d = vector3d(0, 0, 1); // axis aligned, zero components in the inverse
            }
            d.normalize();
            const vector3d o = i % 3 == 0
                                   ? vector3d(pos(rng), pos(rng), pos(rng)) // inside the cloud
                                   : vector3d(-5, pos(rng), pos(rng));
            const ray r(o, d);
            const nearest a = brute_force(boxes, r);
            const nearest b = with_tree(tree, boxes, r);
            CHECK(a.t.has_value() == b.t.has_value());
            if (a.t && b.t) {
                hits++;
                CHECK(*a.t == *b.t);
                CHECK(a.prim == b.prim);
            }
        }
        CHECK(hits > 1000);
    }

    // identical boxes cannot be split and end up in one leaf, still answering correctly
    {
        aabb b;
        b.expand(vector3d(1, 1, 1));
        b.expand(vector3d(2, 2, 2));
        std::vector<aabb> boxes(50, b);
        bvh tree;
        tree.build(boxes);
        CHECK(tree.node_count() == 1);
        const nearest n = with_tree(tree, boxes, ray(vector3d(0, 1.5, 1.5), vector3d(1, 0, 0)));
        CHECK(n.t.has_value() && n.prim == 0);
    }
    return TEST_RESULT();
}
