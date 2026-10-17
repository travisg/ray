#include "check.h"
#include <Ray.h>
#include <drawables/Plane.h>
#include <drawables/Sphere.h>
#include <drawables/Triangle.h>

using Libvec::Vector3d;

int main() {
    // sphere of radius 1 at z = 10, ray down the z axis
    {
        Sphere s(Vector3d(0, 0, 10), 1);
        auto hit = s.Intersect(Ray(Vector3d(0, 0, 0), Vector3d(0, 0, 1)));
        CHECK(hit.has_value());
        if (hit) {
            CHECK_NEAR(hit->t, 9.0, 1e-9);
            CHECK_NEAR(hit->pos.getz(), 9.0, 1e-9);
            CHECK_NEAR(hit->normal.getz(), -1.0, 1e-9);
        }
        // pointing away
        CHECK(!s.Intersect(Ray(Vector3d(0, 0, 0), Vector3d(0, 0, -1))));
        // parallel miss
        CHECK(!s.Intersect(Ray(Vector3d(5, 0, 0), Vector3d(0, 0, 1))));
    }

    // plane z = 0 facing +z
    {
        Plane p(Ray(Vector3d(0, 0, 0), Vector3d(0, 0, 1)));
        auto hit = p.Intersect(Ray(Vector3d(1, 2, 5), Vector3d(0, 0, -1)));
        CHECK(hit.has_value());
        if (hit) {
            CHECK_NEAR(hit->t, 5.0, 1e-9);
            CHECK_NEAR(hit->pos.getx(), 1.0, 1e-9);
            CHECK_NEAR(hit->pos.gety(), 2.0, 1e-9);
            CHECK_NEAR(hit->normal.getz(), 1.0, 1e-9);
        }
        // back face
        CHECK(!p.Intersect(Ray(Vector3d(0, 0, -5), Vector3d(0, 0, 1))));
    }

    // triangle in the z = 0 plane, counter-clockwise so it faces +z
    {
        Triangle t(Vector3d(0, 0, 0), Vector3d(10, 0, 0), Vector3d(0, 10, 0));
        auto hit = t.Intersect(Ray(Vector3d(1, 1, 5), Vector3d(0, 0, -1)));
        CHECK(hit.has_value());
        if (hit) {
            CHECK_NEAR(hit->t, 5.0, 1e-9);
            CHECK_NEAR(hit->normal.getz(), 1.0, 1e-9);
        }
        // outside the triangle
        CHECK(!t.Intersect(Ray(Vector3d(8, 8, 5), Vector3d(0, 0, -1))));
        // back face
        CHECK(!t.Intersect(Ray(Vector3d(1, 1, -5), Vector3d(0, 0, 1))));
    }

    return TEST_RESULT();
}
