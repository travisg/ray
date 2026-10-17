#include "check.h"
#include <cstdio>
#include <iterator>
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

    CHECK(std::distance(g->VertexListIterator(), g->VertexListEnd()) == 8);
    CHECK(std::distance(g->NormalVertexListIterator(), g->NormalVertexListEnd()) == 24);

    const auto meshes = std::distance(g->ListIterator(), g->ListEnd());
    CHECK(meshes == 1);
    if (meshes == 1) {
        const Mesh *m = *g->ListIterator();
        CHECK(m->GetSurfaceList().size() == 6);
        for (const Surface *s : m->GetSurfaceList()) {
            CHECK(s->m_Indexes.size() == 4);
        }
    }

    delete g;
    return TEST_RESULT();
}
