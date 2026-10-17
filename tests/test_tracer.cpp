#include "check.h"
#include <drawables/plane.h>
#include <memory>
#include <render_surface.h>
#include <scene.h>
#include <shaders/default_shader.h>
#include <trace_master.h>
#include <tracer.h>
#include <vector>

using libvec::vector3d;

namespace {

// Keeps the rendered pixels in memory.
class memory_surface : public render_surface {
  public:
    memory_surface(int width, int height)
        : render_surface(width, height), pixels_(size_t(width) * height) {}

    void set_xy(int x, int y, colorf color) override { pixels_[y * width() + x] = color; }
    void set_xy_run(int x, int y, int count, const colorf *color) override {
        for (int i = 0; i < count; i++) {
            set_xy(x + i, y, color[i]);
        }
    }
    const colorf &at(int x, int y) const { return pixels_[y * width() + x]; }

  private:
    std::vector<colorf> pixels_;
};

void render(const scene &s, memory_surface &surface) {
    trace_master master(surface, tile_order::sequential, 1);
    tracer t(surface, s, master);
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
        memory_surface surface(20, 20);
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
        memory_surface surface(20, 20);
        render(s, surface);
        CHECK(surface.at(10, 2).r == 0);
        CHECK(surface.at(10, 18).r > 0);
    }

    return TEST_RESULT();
}
