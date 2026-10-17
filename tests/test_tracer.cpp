#include <cmath>
#include <memory>
#include <ray/drawables/plane.h>
#include <ray/drawables/sphere.h>
#include <ray/framebuffer.h>
#include <ray/sampling.h>
#include <ray/scene.h>
#include <ray/shaders/default_shader.h>
#include <ray/trace_master.h>
#include <ray/tracer.h>
#include <tests/check.h>
#include <vector>

using libvec::vector3d;

namespace {

void render(const scene &s, framebuffer &surface, render_settings settings = {}) {
    trace_master master(surface, tile_order::sequential, 1);
    tracer t(surface, s, master, settings);
    t.trace();
    master.wait_for_done();
}

} // namespace

int main() {
    // a red floor lit from above
    scene s;
    auto shader = std::make_shared<default_shader>();
    shader->set_diffuse_color(colorf(1, 0, 0));
    auto floor = std::make_unique<plane>(ray(vector3d(0, 0, 0), vector3d(0, 0, 1)));
    floor->set_shader(shader);
    s.add_drawable(std::move(floor));
    s.add_light(simple_light(vector3d(0, 0, 10), colorf(1, 1, 1), 100));

    // seen from straight above, where the camera's default up vector is degenerate
    s.set_camera(vector3d(0, 0, 10), vector3d(0, 0, 0));
    s.prepare();
    {
        framebuffer surface(20, 20);
        render(s, surface);
        for (int y = 0; y < 20; y++) {
            for (int x = 0; x < 20; x++) {
                const colorf &c = surface.at(x, y);
                CHECK(c.r > 0.5 && c.g == 0 && c.b == 0);
            }
        }
        // brightest directly below the light
        CHECK(surface.at(10, 10).r > surface.at(0, 0).r);
    }

    // seen from the side at floor level the floor fills the lower half only
    s.set_camera(vector3d(10, 0, 0.5), vector3d(0, 0, 0.5));
    {
        framebuffer surface(20, 20);
        render(s, surface);
        CHECK(surface.at(10, 2).r == 0);
        CHECK(surface.at(10, 18).r > 0);
    }

    // the Halton sequence: digits reflected about the radix point
    CHECK(radical_inverse(0, 2) == 0 && radical_inverse(1, 2) == 0.5 &&
          radical_inverse(2, 2) == 0.25);
    CHECK(radical_inverse(3, 2) == 0.75 && radical_inverse(4, 2) == 0.125);
    CHECK(std::fabs(radical_inverse(1, 3) - 1.0 / 3) < 1e-12);
    CHECK(std::fabs(radical_inverse(3, 3) - 1.0 / 9) < 1e-12);
    CHECK(fract(1.75) == 0.75 && fract(-0.25) == 0.75);

    // passes accumulate into a running mean
    {
        framebuffer fb(2, 1);
        const colorf p0[2] = { colorf(1, 0, 0), colorf(2, 2, 2) };
        const colorf p1[2] = { colorf(0, 1, 0), colorf(0, 0, 0) };
        const colorf p2[2] = { colorf(0, 0, 3), colorf(1, 1, 1) };
        fb.accumulate_run(0, 0, 2, p0, 0);
        fb.accumulate_run(0, 0, 2, p1, 1);
        CHECK(fb.at(0, 0).r == 0.5 && fb.at(0, 0).g == 0.5 && fb.at(0, 0).b == 0);
        CHECK(fb.at(1, 0).r == 1);
        fb.accumulate_run(0, 0, 2, p2, 2);
        CHECK(std::fabs(fb.at(0, 0).b - 1.0) < 1e-6 && std::fabs(fb.at(0, 0).r - 1.0 / 3) < 1e-6);
        CHECK(std::fabs(fb.at(1, 0).g - 1.0) < 1e-6);
    }

    // indirect light: a white floor under a white sky, with a sphere resting on it. Seen from
    // above, the open floor is lit by the whole sky, the floor beside the sphere by less of it,
    // and the sphere's top by all of it; the same seed gives the same picture every time.
    {
        scene sky;
        sky.set_ambient_light(colorf(1, 1, 1));
        auto white = std::make_shared<default_shader>();
        auto floor = std::make_unique<plane>(ray(vector3d(0, 0, 0), vector3d(0, 0, 1)));
        floor->set_shader(white);
        sky.add_drawable(std::move(floor));
        auto ball = std::make_unique<sphere>(vector3d(0, 0, 1), 1);
        ball->set_shader(white);
        sky.add_drawable(std::move(ball));
        sky.set_camera(vector3d(0, 0, 10), vector3d(0, 0, 0));
        sky.prepare();

        render_settings settings;
        settings.gi_samples = 256;
        framebuffer a(40, 40);
        render(sky, a, settings);
        const double open = a.at(2, 2).r;
        const double beside = a.at(20, 23).r; // the floor 1.5 units from the sphere's axis
        const double top = a.at(20, 20).r;
        CHECK(open > 0.9);
        CHECK(beside < open - 0.1);
        CHECK(top > 0.9);

        framebuffer b(40, 40);
        render(sky, b, settings);
        bool same = true;
        for (size_t i = 0; i < a.size(); i++) {
            same = same && a.data()[i] == b.data()[i];
        }
        CHECK(same);
    }

    return TEST_RESULT();
}
