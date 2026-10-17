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
#include <ray/scene_library.h>

#include <cstdio>
#include <iostream>
#include <ray/drawables/mesh_drawable.h>
#include <ray/drawables/plane.h>
#include <ray/drawables/sphere.h>
#include <ray/drawables/triangle.h>
#include <ray/lights/simple_light.h>
#include <ray/mesh/geometry.h>
#include <ray/mesh/obj_loader.h>
#include <ray/ray.h>
#include <ray/scene.h>
#include <ray/shaders/default_shader.h>
#include <string>
#include <utility>

using libvec::vector3d;

namespace {

using scene_builder = bool (*)(scene &, std::mt19937 &);

struct scene_entry {
    scene_info info;
    scene_builder build;
};

shader_ptr make_shader(colorf diffuse, double shiny) {
    auto s = std::make_shared<default_shader>();
    s->set_diffuse_color(diffuse);
    s->set_shinyness(shiny);
    return s;
}

template <typename T, typename... Args> void add(scene &s, shader_ptr shader, Args &&...args) {
    auto d = std::make_unique<T>(std::forward<Args>(args)...);
    d->set_shader(std::move(shader));
    s.add_drawable(std::move(d));
}

// Loads an .obj, scaled about the origin and then moved by offset.
std::unique_ptr<mesh_drawable> load_mesh(const std::string &filename, double scale = 1.0,
                                         const vector3d &offset = vector3d(0.0)) {
    std::unique_ptr<geometry> geom = load_obj_file(filename);
    if (!geom) {
        return nullptr;
    }
    geom->transform(scale, offset);
    size_t faces = 0;
    for (const auto &m : geom->meshes()) {
        faces += m->surfaces().size();
    }
    printf("loaded %s: %zu vertices, %zu faces\n", filename.c_str(), geom->vertices().size(),
           faces);
    return std::make_unique<mesh_drawable>(std::move(geom));
}

// A single mesh at the origin, lit by two white lights, viewed from (2, 2, 2).
bool build_mesh_scene(scene &s, const char *asset, colorf diffuse) {
    auto md = load_mesh(std::string("resources/") + asset);
    if (!md) {
        return false;
    }
    md->set_shader(make_shader(diffuse, 0.0));
    s.add_drawable(std::move(md));
    s.add_light(simple_light(vector3d(20.0, 10.0, 20), colorf(1.0, 1.0, 1.0), 100));
    s.add_light(simple_light(vector3d(0.0, 0.0, 20), colorf(1.0, 1.0, 1.0), 100));
    s.set_camera(vector3d(2.0, 2.0, 2.0), vector3d(0.0, 0.0, 0.0));
    return true;
}

bool build_default(scene &s, std::mt19937 &) {
    s.set_ambient_light(colorf(0.0, 0.0, 0.0));
    add<sphere>(s, make_shader(colorf(1.0, 1.0, 1.0), 0.8), vector3d(10.0, 10.0, 10.0), 8.0);
    // floor and two walls meeting at the origin
    add<plane>(s, make_shader(colorf(1.0, 0.2, 0.2), 0.5),
               ray(vector3d(0.0, 0.0, 0.0), vector3d(0.0, 0.0, 1.0)));
    add<plane>(s, make_shader(colorf(0.2, 1.0, 0.2), 0.5),
               ray(vector3d(0.0, 0.0, 0.0), vector3d(0.0, 1.0, 0.0)));
    add<plane>(s, make_shader(colorf(0.2, 0.2, 1.0), 0.5),
               ray(vector3d(0.0, 0.0, 0.0), vector3d(1.0, 0.0, 0.0)));
    s.add_light(simple_light(vector3d(20.0, 10.0, 20), colorf(1.0, 1.0, 1.0), 100));
    s.add_light(simple_light(vector3d(0.0, 0.0, 20), colorf(1.0, 1.0, 1.0), 100));
    s.set_camera(vector3d(20.0, 20.0, 20.0), vector3d(0.0, 0.0, 0.0));
    return true;
}

bool build_spheres(scene &s, std::mt19937 &rng) {
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
        add<sphere>(s, make_shader(diffuse, 0.0), vector3d(x, y, z), radius(rng));
    }
    // the enclosing sphere and two planes behind it
    add<sphere>(s, make_shader(colorf(1.0, 1.0, 1.0), 0.0), vector3d(0.0, 0.0, 0.0), 50.0);
    add<plane>(s, make_shader(colorf(1.0, 1.0, 1.0), 0.2),
               ray(vector3d(0.0, 0.0, -50.0), vector3d(0.0, 0.0, 1.0)));
    add<plane>(s, make_shader(colorf(1.0, 1.0, 1.0), 0.0),
               ray(vector3d(-50.0, 0.0, 0.0), vector3d(1.0, 0.0, 0.0)));
    s.add_light(simple_light(vector3d(0.0, 0.0, 100.0), colorf(1.0, 0.0, 0.0), 100));
    s.add_light(simple_light(vector3d(100.0, 0.0, 5.0), colorf(0.0, 1.0, 0.0), 100));
    s.add_light(simple_light(vector3d(0.0, 100.0, 5.0), colorf(0.0, 0.0, 1.0), 100));
    s.add_light(simple_light(vector3d(0.0, 60.0, 60.0), colorf(1.0, 1.0, 1.0), 100));
    s.add_light(simple_light(vector3d(0.0, 0.0, 20), colorf(1.0, 1.0, 1.0), 100));
    s.set_camera(vector3d(20.0, 20.0, 20.0), vector3d(0.0, 0.0, 0.0));
    return true;
}

bool build_triangle(scene &s, std::mt19937 &) {
    s.set_ambient_light(colorf(0.0, 0.0, 0.2));
    add<triangle>(s, make_shader(colorf(0.2, 0.2, 1.0), 0.5), vector3d(0.0, 0.0, 10.0),
                  vector3d(20.0, 0.0, 0.0), vector3d(0.0, 20.0, 0.0));
    add<plane>(s, make_shader(colorf(1.0, 0.0, 0.0), 0.2),
               ray(vector3d(0.0, 0.0, -3.0), vector3d(0.0, 0.0, 1.0)));
    s.add_light(simple_light(vector3d(1.0, 100.0, 5.0), colorf(0.0, 0.0, 1.0), 100));
    s.add_light(simple_light(vector3d(0.0, 0.0, 20), colorf(1.0, 1.0, 1.0), 100));
    s.set_camera(vector3d(20.0, 20.0, 20.0), vector3d(0.0, 0.0, 0.0));
    return true;
}

bool build_cbox(scene &s, std::mt19937 &) {
    return build_mesh_scene(s, "cbox.obj", colorf(1.0, 1.0, 0.0));
}

bool build_dude(scene &s, std::mt19937 &) {
    return build_mesh_scene(s, "dude.obj", colorf(1.0, 1.0, 0.0));
}

bool build_plane_mesh(scene &s, std::mt19937 &) {
    return build_mesh_scene(s, "plane.obj", colorf(0.0, 0.0, 1.0));
}

bool build_csphere(scene &s, std::mt19937 &) {
    return build_mesh_scene(s, "csphere.obj", colorf(0.0, 0.0, 1.0));
}

bool build_blob(scene &s, std::mt19937 &) {
    return build_mesh_scene(s, "blob.obj", colorf(1.0, 0.5, 0.5));
}

// The 256x256 heightfield of map.obj spans 65536 units; scaled to 16 across, with its
// relief about 1.6 tall, under one low sun and a touch of ambient so the shadows stay dark.
constexpr double kMapScale = 1.0 / 4096.0;

bool add_map(scene &s) {
    auto md = load_mesh("resources/map.obj", kMapScale);
    if (!md) {
        return false;
    }
    md->set_shader(make_shader(colorf(0.35, 0.35, 0.25), 0.0));
    s.add_drawable(std::move(md));
    s.add_light(simple_light(vector3d(-40.0, -30.0, 20.0), colorf(1.0, 1.0, 1.0), 100));
    s.set_ambient_light(colorf(0.05, 0.05, 0.05));
    return true;
}

bool build_map(scene &s, std::mt19937 &) {
    if (!add_map(s)) {
        return false;
    }
    s.set_camera(vector3d(9.0, -9.0, 5.0), vector3d(0.0, 0.0, 0.0));
    return true;
}

// The blob, doubled, floating above the map, so its shadow falls on the terrain and the
// terrain shows in its reflection.
bool build_map_blob(scene &s, std::mt19937 &) {
    if (!add_map(s)) {
        return false;
    }
    auto blob = load_mesh("resources/blob.obj", 2.0, vector3d(0.0, 0.0, 3.0));
    if (!blob) {
        return false;
    }
    blob->set_shader(make_shader(colorf(0.9, 0.0, 0.0), 0.3));
    s.add_drawable(std::move(blob));
    s.set_camera(vector3d(8.0, -8.0, 4.5), vector3d(0.0, 0.0, 1.5));
    return true;
}

constexpr scene_entry kScenes[] = {
    { { "default", "a shiny sphere in the corner of three coloured walls" }, build_default },
    { { "spheres", "100 random spheres inside a 50-unit sphere, camera inside too" },
      build_spheres },
    { { "triangle", "one triangle over a red plane" }, build_triangle },
    { { "cbox", "resources/cbox.obj, a cube of six quads" }, build_cbox },
    { { "dude", "resources/dude.obj" }, build_dude },
    { { "plane-mesh", "resources/plane.obj, a triangulated plane" }, build_plane_mesh },
    { { "csphere", "resources/csphere.obj, a coarse sphere" }, build_csphere },
    { { "blob", "resources/blob.obj, 13824 quads" }, build_blob },
    { { "map", "resources/map.obj, a 256x256 heightfield of 131072 triangles" }, build_map },
    { { "map-blob", "the blob floating above the map" }, build_map_blob },
};

constexpr scene_info kSceneInfos[] = {
    kScenes[0].info, kScenes[1].info, kScenes[2].info, kScenes[3].info, kScenes[4].info,
    kScenes[5].info, kScenes[6].info, kScenes[7].info, kScenes[8].info, kScenes[9].info,
};
static_assert(std::size(kSceneInfos) == std::size(kScenes));

} // namespace

std::span<const scene_info> list_scenes() {
    return kSceneInfos;
}

std::unique_ptr<scene> build_scene(std::string_view name, std::mt19937 &rng) {
    for (const scene_entry &entry : kScenes) {
        if (entry.info.name != name) {
            continue;
        }
        auto s = std::make_unique<scene>();
        if (!entry.build(*s, rng)) {
            return nullptr;
        }
        s->prepare();
        return s;
    }
    std::cerr << "unknown scene '" << name << "' (see --list-scenes)" << std::endl;
    return nullptr;
}
