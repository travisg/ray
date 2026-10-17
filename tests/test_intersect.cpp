#include "check.h"
#include <Ray.h>
#include <Scene.h>
#include <drawables/Plane.h>
#include <drawables/Sphere.h>
#include <drawables/Triangle.h>
#include <memory>

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

    // a scene reports the nearest hit and which drawable it came from
    {
        Scene scene;
        auto sphere = std::make_unique<Sphere>(Vector3d(0, 0, 10), 1);
        const Drawable *sp = sphere.get();
        scene.AddDrawable(std::move(sphere));
        scene.AddDrawable(std::make_unique<Plane>(Ray(Vector3d(0, 0, 20), Vector3d(0, 0, -1))));
        scene.Prepare();

        auto hit = scene.Intersect(Ray(Vector3d(0, 0, 0), Vector3d(0, 0, 1)));
        CHECK(hit.has_value());
        if (hit) {
            CHECK(hit->drawable == sp);
            CHECK_NEAR(hit->t, 9.0, 1e-9);
        }
        auto behind = scene.Intersect(Ray(Vector3d(5, 0, 0), Vector3d(0, 0, 1)));
        CHECK(behind.has_value());
        if (behind) {
            CHECK(behind->drawable != sp);
            CHECK_NEAR(behind->t, 20.0, 1e-9);
        }
        CHECK(scene.DoesIntersect(Ray(Vector3d(0, 0, 0), Vector3d(0, 0, 1)), 15));
        CHECK(!scene.DoesIntersect(Ray(Vector3d(0, 0, 0), Vector3d(0, 0, 1)), 5));
    }

    return TEST_RESULT();
}
