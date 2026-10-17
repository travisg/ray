#include "check.h"
#include <drawables/plane.h>
#include <drawables/sphere.h>
#include <drawables/triangle.h>
#include <memory>
#include <ray.h>
#include <scene.h>

using libvec::vector3d;

int main() {
    // sphere of radius 1 at z = 10, ray down the z axis
    {
        sphere s(vector3d(0, 0, 10), 1);
        auto hit = s.intersect(ray(vector3d(0, 0, 0), vector3d(0, 0, 1)));
        CHECK(hit.has_value());
        if (hit) {
            CHECK_NEAR(hit->t, 9.0, 1e-9);
            CHECK_NEAR(hit->pos.z, 9.0, 1e-9);
            CHECK_NEAR(hit->normal.z, -1.0, 1e-9);
        }
        // pointing away
        CHECK(!s.intersect(ray(vector3d(0, 0, 0), vector3d(0, 0, -1))));
        // parallel miss
        CHECK(!s.intersect(ray(vector3d(5, 0, 0), vector3d(0, 0, 1))));
        // from inside: the far side, with the normal facing back in
        auto inner = s.intersect(ray(vector3d(0, 0, 10), vector3d(0, 0, 1)));
        CHECK(inner.has_value());
        if (inner) {
            CHECK_NEAR(inner->t, 1.0, 1e-9);
            CHECK_NEAR(inner->pos.z, 11.0, 1e-9);
            CHECK_NEAR(inner->normal.z, -1.0, 1e-9);
        }
    }

    // plane z = 0 facing +z
    {
        plane p(ray(vector3d(0, 0, 0), vector3d(0, 0, 1)));
        auto hit = p.intersect(ray(vector3d(1, 2, 5), vector3d(0, 0, -1)));
        CHECK(hit.has_value());
        if (hit) {
            CHECK_NEAR(hit->t, 5.0, 1e-9);
            CHECK_NEAR(hit->pos.x, 1.0, 1e-9);
            CHECK_NEAR(hit->pos.y, 2.0, 1e-9);
            CHECK_NEAR(hit->normal.z, 1.0, 1e-9);
        }
        // back face
        CHECK(!p.intersect(ray(vector3d(0, 0, -5), vector3d(0, 0, 1))));
        // behind the origin
        CHECK(!p.intersect(ray(vector3d(0, 0, -5), vector3d(0, 0, -1))));
        // parallel
        CHECK(!p.intersect(ray(vector3d(0, 0, 5), vector3d(1, 0, 0))));
    }

    // triangle in the z = 0 plane, counter-clockwise so it faces +z
    {
        triangle t(vector3d(0, 0, 0), vector3d(10, 0, 0), vector3d(0, 10, 0));
        auto hit = t.intersect(ray(vector3d(1, 1, 5), vector3d(0, 0, -1)));
        CHECK(hit.has_value());
        if (hit) {
            CHECK_NEAR(hit->t, 5.0, 1e-9);
            CHECK_NEAR(hit->normal.z, 1.0, 1e-9);
        }
        // outside the triangle
        CHECK(!t.intersect(ray(vector3d(8, 8, 5), vector3d(0, 0, -1))));
        // back face
        CHECK(!t.intersect(ray(vector3d(1, 1, -5), vector3d(0, 0, 1))));
        // behind the origin
        CHECK(!t.intersect(ray(vector3d(1, 1, -5), vector3d(0, 0, -1))));
    }

    // a scene reports the nearest hit and which drawable it came from
    {
        scene s;
        auto ball = std::make_unique<sphere>(vector3d(0, 0, 10), 1);
        const drawable *sp = ball.get();
        s.add_drawable(std::move(ball));
        s.add_drawable(std::make_unique<plane>(ray(vector3d(0, 0, 20), vector3d(0, 0, -1))));
        s.prepare();

        auto hit = s.intersect(ray(vector3d(0, 0, 0), vector3d(0, 0, 1)));
        CHECK(hit.has_value());
        if (hit) {
            CHECK(hit->drawable == sp);
            CHECK_NEAR(hit->t, 9.0, 1e-9);
        }
        auto behind = s.intersect(ray(vector3d(5, 0, 0), vector3d(0, 0, 1)));
        CHECK(behind.has_value());
        if (behind) {
            CHECK(behind->drawable != sp);
            CHECK_NEAR(behind->t, 20.0, 1e-9);
        }
        CHECK(s.does_intersect(ray(vector3d(0, 0, 0), vector3d(0, 0, 1)), 15));
        CHECK(!s.does_intersect(ray(vector3d(0, 0, 0), vector3d(0, 0, 1)), 5));
        // an occluder exactly at the end of the shadow ray does not count
        CHECK(!s.does_intersect(ray(vector3d(5, 0, 0), vector3d(0, 0, 1)), 20));
        CHECK(s.does_intersect(ray(vector3d(5, 0, 0), vector3d(0, 0, 1)), 25));
        // a hit at the origin itself is ignored
        CHECK(!s.intersect(ray(vector3d(0, 0, 20), vector3d(0, 0, 1))));
    }

    return TEST_RESULT();
}
