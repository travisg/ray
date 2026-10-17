#include "check.h"
#include <memory>
#include <mesh/Geometry.h>
#include <mesh/OBJLoader.h>
#include <sstream>

int main() {
    // cbox.obj: a cube of 8 vertices, 24 normals and 6 quad faces in one mesh
    {
        std::unique_ptr<Geometry> g = LoadObjFile(RAY_SOURCE_DIR "/resources/cbox.obj");
        CHECK(g != nullptr);
        if (g) {
            CHECK(g->Vertices().size() == 8);
            CHECK(g->Normals().size() == 24);
            CHECK(g->Meshes().size() == 1);
            if (g->Meshes().size() == 1) {
                const Mesh *m = g->Meshes()[0].get();
                CHECK(m->GetName() == "default0");
                CHECK(m->GetSurfaceList().size() == 6);
                for (const Surface &s : m->GetSurfaceList()) {
                    CHECK(s.m_Indexes.size() == 4);
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
        std::unique_ptr<Geometry> g = LoadObj(in);
        CHECK(g != nullptr);
        if (g) {
            CHECK(g->Vertices().size() == 7);
            CHECK(g->Meshes().size() == 2);
            if (g->Meshes().size() == 2) {
                CHECK(g->Meshes()[0]->GetName() == "lid");
                CHECK(g->Meshes()[0]->GetSurfaceList().size() == 1);
                CHECK(g->Meshes()[1]->GetName() == "default1");
                CHECK(g->Meshes()[1]->GetSurfaceList().size() == 1);
                const Surface &quad = g->Meshes()[1]->GetSurfaceList()[0];
                CHECK(quad.m_Indexes.size() == 4);
                CHECK(quad.m_Indexes[0].posIndex == 3);
                CHECK(quad.m_Indexes[0].uvIndex == 0);
                CHECK(quad.m_Indexes[0].normIndex == 0);
                const Surface &tri = g->Meshes()[0]->GetSurfaceList()[0];
                CHECK(tri.m_Indexes[1].uvIndex == -1);
            }
        }
    }

    // a face without normals, a face before any vertex, and a bad index all fail
    {
        std::istringstream in("v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n");
        CHECK(LoadObj(in) == nullptr);
    }
    {
        std::istringstream in("f 1 2 3\n");
        CHECK(LoadObj(in) == nullptr);
    }
    {
        std::istringstream in("v 0 0 0\nv 1 0 0\nv 0 1 0\nvn 0 0 1\nf 1//1 2//1 9//1\n");
        CHECK(LoadObj(in) == nullptr);
    }
    CHECK(LoadObjFile(RAY_SOURCE_DIR "/resources/missing.obj") == nullptr);

    return TEST_RESULT();
}
