#include <memory>
#include <ray/mesh/geometry.h>
#include <ray/mesh/obj_loader.h>
#include <sstream>
#include <tests/check.h>

int main() {
    // cbox.obj: a cube of 8 vertices, 24 normals and 6 quad faces in one mesh
    {
        std::unique_ptr<geometry> g = load_obj_file(RAY_SOURCE_DIR "/resources/cbox.obj");
        CHECK(g != nullptr);
        if (g) {
            CHECK(g->vertices().size() == 8);
            CHECK(g->normals().size() == 24);
            CHECK(g->meshes().size() == 1);
            if (g->meshes().size() == 1) {
                const mesh *m = g->meshes()[0].get();
                CHECK(m->name() == "default0");
                CHECK(m->surfaces().size() == 6);
                for (const surface &s : m->surfaces()) {
                    CHECK(s.indexes.size() == 4);
                }
            }
        }
    }

    // a//c vertices, CRLF lines, a named second mesh, and a skipped 5-gon
    {
        std::istringstream in("#  ExternalName: lid\r\n"
                              "v 0 0 0\r\nv 1 0 0\r\nv 0 1 0\r\n"
                              "vn 0 0 1\r\n"
                              "f 1//1 2//1 3//1\r\n"
                              "f 1//1 2//1 3//1 2//1 1//1\r\n"
                              "v 0 0 1\r\nv 1 0 1\r\nv 0 1 1\r\nv 1 1 1\r\n"
                              "vt 0.5 0.5\r\n"
                              "f 4/1/1 5/1/1 7/1/1 6/1/1\r\n");
        std::unique_ptr<geometry> g = load_obj(in);
        CHECK(g != nullptr);
        if (g) {
            CHECK(g->vertices().size() == 7);
            CHECK(g->meshes().size() == 2);
            if (g->meshes().size() == 2) {
                CHECK(g->meshes()[0]->name() == "lid");
                CHECK(g->meshes()[0]->surfaces().size() == 1);
                CHECK(g->meshes()[1]->name() == "default1");
                CHECK(g->meshes()[1]->surfaces().size() == 1);
                const surface &quad = g->meshes()[1]->surfaces()[0];
                CHECK(quad.indexes.size() == 4);
                CHECK(quad.indexes[0].pos_index == 3);
                CHECK(quad.indexes[0].uv_index == 0);
                CHECK(quad.indexes[0].norm_index == 0);
                const surface &tri = g->meshes()[0]->surfaces()[0];
                CHECK(tri.indexes[1].uv_index == -1);
            }
        }
    }

    // a face without normals, a face before any vertex, and a bad index all fail
    {
        std::istringstream in("v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n");
        CHECK(load_obj(in) == nullptr);
    }
    {
        std::istringstream in("f 1 2 3\n");
        CHECK(load_obj(in) == nullptr);
    }
    {
        std::istringstream in("v 0 0 0\nv 1 0 0\nv 0 1 0\nvn 0 0 1\nf 1//1 2//1 9//1\n");
        CHECK(load_obj(in) == nullptr);
    }
    CHECK(load_obj_file(RAY_SOURCE_DIR "/resources/missing.obj") == nullptr);

    return TEST_RESULT();
}
