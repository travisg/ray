#include "check.h"
#include <RenderSurface.h>
#include <Scene.h>
#include <TraceMaster.h>
#include <Tracer.h>
#include <drawables/Plane.h>
#include <memory>
#include <shaders/DefaultShader.h>
#include <vector>

using Libvec::Vector3d;

namespace {

// Keeps the rendered pixels in memory.
class MemorySurface : public RenderSurface {
  public:
    MemorySurface(int width, int height)
        : RenderSurface(width, height), m_pixels(size_t(width) * height) {}

    void SetXY(int x, int y, colorf color) override { m_pixels[y * Width() + x] = color; }
    void SetXYRun(int x, int y, int count, const colorf *color) override {
        for (int i = 0; i < count; i++) {
            SetXY(x + i, y, color[i]);
        }
    }
    const colorf &At(int x, int y) const { return m_pixels[y * Width() + x]; }

  private:
    std::vector<colorf> m_pixels;
};

void Render(const Scene &scene, MemorySurface &surface) {
    TraceMaster master(surface, TileOrder::Sequential, 1);
    Tracer tracer(surface, scene, master);
    tracer.Trace();
    master.WaitForDone();
}

} // namespace

int main() {
    // a red floor lit from above
    Scene scene;
    auto shader = std::make_shared<DefaultShader>();
    shader->SetDiffuseColor(colorf(1, 0, 0));
    auto floor = std::make_unique<Plane>(Ray(Vector3d(0, 0, 0), Vector3d(0, 0, 1)));
    floor->SetShader(shader);
    scene.AddDrawable(std::move(floor));
    scene.AddLight(SimpleLight(Vector3d(0, 0, 10), colorf(1, 1, 1), 100));

    // seen from straight above, where the camera's default up vector is degenerate
    scene.SetCamera(Vector3d(0, 0, 10), Vector3d(0, 0, 0));
    scene.Prepare();
    {
        MemorySurface surface(20, 20);
        Render(scene, surface);
        for (int y = 0; y < 20; y++) {
            for (int x = 0; x < 20; x++) {
                const colorf &c = surface.At(x, y);
                CHECK(c.r > 0.5 && c.g == 0 && c.b == 0);
            }
        }
        // brightest directly below the light
        CHECK(surface.At(10, 10).r > surface.At(0, 0).r);
    }

    // seen from the side at floor level the floor fills the lower half only
    scene.SetCamera(Vector3d(10, 0, 0.5), Vector3d(0, 0, 0.5));
    {
        MemorySurface surface(20, 20);
        Render(scene, surface);
        CHECK(surface.At(10, 2).r == 0);
        CHECK(surface.At(10, 18).r > 0);
    }

    return TEST_RESULT();
}
