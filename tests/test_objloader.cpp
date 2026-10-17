#include "check.h"
#include <cstdio>
#include <mesh/Geometry.h>
#include <mesh/OBJLoader.h>

// cbox.obj: a cube of 8 vertices, 24 normals and 6 quad faces in one mesh.
int main() {
    FILE *fp = std::fopen(RAY_SOURCE_DIR "/resources/cbox.obj", "r");
    CHECK(fp != nullptr);
    if (!fp) {
        return 1;
    }

    Geometry *g = nullptr;
    CHECK(obj_load(fp, &g) == 0);
    std::fclose(fp);
    CHECK(g != nullptr);
    if (!g) {
        return 1;
    }

    CHECK(g->Vertices().size() == 8);
    CHECK(g->Normals().size() == 24);

    CHECK(g->Meshes().size() == 1);
    if (g->Meshes().size() == 1) {
        const Mesh *m = g->Meshes()[0];
        CHECK(m->GetSurfaceList().size() == 6);
        for (const Surface *s : m->GetSurfaceList()) {
            CHECK(s->m_Indexes.size() == 4);
        }
    }

    delete g;
    return TEST_RESULT();
}
