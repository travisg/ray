/*
 * Copyright (c) 2008-2011 Travis Geiselbrecht
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
#include <scene_library.h>

#include <ray.h>
#include <scene.h>
#include <drawables/mesh_drawable.h>
#include <drawables/plane.h>
#include <drawables/sphere.h>
#include <drawables/triangle.h>
#include <iostream>
#include <lights/simple_light.h>
#include <mesh/geometry.h>
#include <mesh/obj_loader.h>
#include <shaders/default_shader.h>
#include <string>
#include <utility>

using Libvec::Vector3d;

namespace {

using SceneBuilder = bool (*)(Scene &scene, std::mt19937 &rng);

struct SceneEntry {
    SceneInfo info;
    SceneBuilder build;
};

ShaderPtr MakeShader(colorf diffuse, double shiny) {
    auto s = std::make_shared<DefaultShader>();
    s->SetDiffuseColor(diffuse);
    s->SetShinyness(shiny);
    return s;
}

template <typename T, typename... Args> void Add(Scene &scene, ShaderPtr shader, Args &&...args) {
    auto d = std::make_unique<T>(std::forward<Args>(args)...);
    d->SetShader(std::move(shader));
    scene.AddDrawable(std::move(d));
}

std::unique_ptr<MeshDrawable> LoadMesh(const std::string &filename) {
    std::cout << "loading " << filename << std::endl;
    std::unique_ptr<Geometry> geom = LoadObjFile(filename);
    if (!geom) {
        return nullptr;
    }
    geom->Dump();
    return std::make_unique<MeshDrawable>(std::move(geom));
}

// A single mesh at the origin, lit by two white lights, viewed from (2, 2, 2).
bool BuildMeshScene(Scene &scene, const char *asset, colorf diffuse) {
    auto md = LoadMesh(std::string("resources/") + asset);
    if (!md) {
        return false;
    }
    md->SetShader(MakeShader(diffuse, 0.0));
    scene.AddDrawable(std::move(md));
    scene.AddLight(SimpleLight(Vector3d(20.0, 10.0, 20), colorf(1.0, 1.0, 1.0), 100));
    scene.AddLight(SimpleLight(Vector3d(0.0, 0.0, 20), colorf(1.0, 1.0, 1.0), 100));
    scene.SetCamera(Vector3d(2.0, 2.0, 2.0), Vector3d(0.0, 0.0, 0.0));
    return true;
}

bool BuildDefault(Scene &scene, std::mt19937 &) {
    scene.SetAmbientLight(colorf(0.0, 0.0, 0.0));
    Add<Sphere>(scene, MakeShader(colorf(1.0, 1.0, 1.0), 0.8), Vector3d(10.0, 10.0, 10.0), 8.0);
    // floor and two walls meeting at the origin
    Add<Plane>(scene, MakeShader(colorf(1.0, 0.2, 0.2), 0.5),
               Ray(Vector3d(0.0, 0.0, 0.0), Vector3d(0.0, 0.0, 1.0)));
    Add<Plane>(scene, MakeShader(colorf(0.2, 1.0, 0.2), 0.5),
               Ray(Vector3d(0.0, 0.0, 0.0), Vector3d(0.0, 1.0, 0.0)));
    Add<Plane>(scene, MakeShader(colorf(0.2, 0.2, 1.0), 0.5),
               Ray(Vector3d(0.0, 0.0, 0.0), Vector3d(1.0, 0.0, 0.0)));
    scene.AddLight(SimpleLight(Vector3d(20.0, 10.0, 20), colorf(1.0, 1.0, 1.0), 100));
    scene.AddLight(SimpleLight(Vector3d(0.0, 0.0, 20), colorf(1.0, 1.0, 1.0), 100));
    scene.SetCamera(Vector3d(20.0, 20.0, 20.0), Vector3d(0.0, 0.0, 0.0));
    return true;
}

bool BuildSpheres(Scene &scene, std::mt19937 &rng) {
    std::uniform_real_distribution<double> position(-50, 50);
    std::uniform_real_distribution<double> radius(1, 20);
    std::uniform_real_distribution<double> channel(0, 1);
    for (int i = 0; i < 100; i++) {
        // one draw per statement: argument evaluation order is unspecified
        const double r = channel(rng);
        const double g = channel(rng);
        const double b = channel(rng);
        const colorf diffuse(r, g, b);
        double x = position(rng);
        double y = position(rng);
        double z = position(rng);
        Add<Sphere>(scene, MakeShader(diffuse, 0.0), Vector3d(x, y, z), radius(rng));
    }
    // the enclosing sphere and two planes behind it
    Add<Sphere>(scene, MakeShader(colorf(1.0, 1.0, 1.0), 0.0), Vector3d(0.0, 0.0, 0.0), 50.0);
    Add<Plane>(scene, MakeShader(colorf(1.0, 1.0, 1.0), 0.2),
               Ray(Vector3d(0.0, 0.0, -50.0), Vector3d(0.0, 0.0, 1.0)));
    Add<Plane>(scene, MakeShader(colorf(1.0, 1.0, 1.0), 0.0),
               Ray(Vector3d(-50.0, 0.0, 0.0), Vector3d(1.0, 0.0, 0.0)));
    scene.AddLight(SimpleLight(Vector3d(0.0, 0.0, 100.0), colorf(1.0, 0.0, 0.0), 100));
    scene.AddLight(SimpleLight(Vector3d(100.0, 0.0, 5.0), colorf(0.0, 1.0, 0.0), 100));
    scene.AddLight(SimpleLight(Vector3d(0.0, 100.0, 5.0), colorf(0.0, 0.0, 1.0), 100));
    scene.AddLight(SimpleLight(Vector3d(0.0, 60.0, 60.0), colorf(1.0, 1.0, 1.0), 100));
    scene.AddLight(SimpleLight(Vector3d(0.0, 0.0, 20), colorf(1.0, 1.0, 1.0), 100));
    scene.SetCamera(Vector3d(20.0, 20.0, 20.0), Vector3d(0.0, 0.0, 0.0));
    return true;
}

bool BuildTriangle(Scene &scene, std::mt19937 &) {
    scene.SetAmbientLight(colorf(0.0, 0.0, 0.2));
    Add<Triangle>(scene, MakeShader(colorf(0.2, 0.2, 1.0), 0.5), Vector3d(0.0, 0.0, 10.0),
                  Vector3d(20.0, 0.0, 0.0), Vector3d(0.0, 20.0, 0.0));
    Add<Plane>(scene, MakeShader(colorf(1.0, 0.0, 0.0), 0.2),
               Ray(Vector3d(0.0, 0.0, -3.0), Vector3d(0.0, 0.0, 1.0)));
    scene.AddLight(SimpleLight(Vector3d(1.0, 100.0, 5.0), colorf(0.0, 0.0, 1.0), 100));
    scene.AddLight(SimpleLight(Vector3d(0.0, 0.0, 20), colorf(1.0, 1.0, 1.0), 100));
    scene.SetCamera(Vector3d(20.0, 20.0, 20.0), Vector3d(0.0, 0.0, 0.0));
    return true;
}

bool BuildCbox(Scene &scene, std::mt19937 &) {
    return BuildMeshScene(scene, "cbox.obj", colorf(1.0, 1.0, 0.0));
}

bool BuildDude(Scene &scene, std::mt19937 &) {
    return BuildMeshScene(scene, "dude.obj", colorf(1.0, 1.0, 0.0));
}

bool BuildPlaneMesh(Scene &scene, std::mt19937 &) {
    return BuildMeshScene(scene, "plane.obj", colorf(0.0, 0.0, 1.0));
}

bool BuildCsphere(Scene &scene, std::mt19937 &) {
    return BuildMeshScene(scene, "csphere.obj", colorf(0.0, 0.0, 1.0));
}

bool BuildBlob(Scene &scene, std::mt19937 &) {
    return BuildMeshScene(scene, "blob.obj", colorf(1.0, 0.5, 0.5));
}

constexpr SceneEntry kScenes[] = {
    { { "default", "a shiny sphere in the corner of three coloured walls" }, BuildDefault },
    { { "spheres", "100 random spheres inside a 50-unit sphere, camera inside too" },
      BuildSpheres },
    { { "triangle", "one triangle over a red plane" }, BuildTriangle },
    { { "cbox", "resources/cbox.obj, a cube of six quads" }, BuildCbox },
    { { "dude", "resources/dude.obj" }, BuildDude },
    { { "plane-mesh", "resources/plane.obj, a triangulated plane" }, BuildPlaneMesh },
    { { "csphere", "resources/csphere.obj, a coarse sphere" }, BuildCsphere },
    { { "blob", "resources/blob.obj, 13824 quads (slow without an acceleration structure)" },
      BuildBlob },
};

constexpr SceneInfo kSceneInfos[] = {
    kScenes[0].info, kScenes[1].info, kScenes[2].info, kScenes[3].info,
    kScenes[4].info, kScenes[5].info, kScenes[6].info, kScenes[7].info,
};
static_assert(std::size(kSceneInfos) == std::size(kScenes));

} // namespace

std::span<const SceneInfo> ListScenes() {
    return kSceneInfos;
}

std::unique_ptr<Scene> BuildScene(std::string_view name, std::mt19937 &rng) {
    for (const SceneEntry &entry : kScenes) {
        if (entry.info.name != name) {
            continue;
        }
        auto scene = std::make_unique<Scene>();
        if (!entry.build(*scene, rng)) {
            return nullptr;
        }
        scene->Prepare();
        return scene;
    }
    std::cerr << "unknown scene '" << name << "' (see --list-scenes)" << std::endl;
    return nullptr;
}
