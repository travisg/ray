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
#include "obj_loader.h"
#include "geometry.h"
#include "mesh.h"
#include <cerrno>
#include <charconv>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

// Fields between runs of spaces; empty fields are dropped.
std::vector<std::string_view> SplitSpaces(std::string_view s) {
    std::vector<std::string_view> out;
    size_t pos = 0;
    while (pos < s.size()) {
        size_t start = s.find_first_not_of(' ', pos);
        if (start == std::string_view::npos) {
            break;
        }
        size_t end = s.find(' ', start);
        if (end == std::string_view::npos) {
            end = s.size();
        }
        out.push_back(s.substr(start, end - start));
        pos = end;
    }
    return out;
}

// Fields between slashes; empty fields are kept, so "1//3" is {"1", "", "3"}.
std::vector<std::string_view> SplitSlashes(std::string_view s) {
    std::vector<std::string_view> out;
    for (;;) {
        size_t slash = s.find('/');
        if (slash == std::string_view::npos) {
            out.push_back(s);
            return out;
        }
        out.push_back(s.substr(0, slash));
        s.remove_prefix(slash + 1);
    }
}

// Reads up to count floats from the fields after the directive; missing ones
// are left untouched. The fields are views into a null-terminated string.
void ParseFloats(const std::vector<std::string_view> &fields, int count, float *out) {
    for (int i = 0; i < count && i + 1 < (int)fields.size(); i++) {
        out[i] = std::strtof(fields[i + 1].data(), nullptr);
    }
}

// 1-based .obj index to 0-based; an empty field becomes -1 (absent).
bool ParseIndex(std::string_view field, int &out) {
    if (field.empty()) {
        out = -1;
        return true;
    }
    int v = 0;
    auto [end, ec] = std::from_chars(field.data(), field.data() + field.size(), v);
    if (ec != std::errc() || end != field.data() + field.size()) {
        return false;
    }
    out = v - 1;
    return true;
}

bool ValidIndex(int index, size_t count) {
    return index >= 0 && static_cast<size_t>(index) < count;
}

// Every surface index must point into its list; uv may be absent.
bool Validate(const Geometry &g) {
    for (const auto &mesh : g.Meshes()) {
        size_t n = 0;
        for (const Surface &s : mesh->GetSurfaceList()) {
            for (const SurfaceIndex &i : s.m_Indexes) {
                if (!ValidIndex(i.posIndex, g.Vertices().size()) ||
                    !ValidIndex(i.normIndex, g.Normals().size()) ||
                    (i.uvIndex != -1 && !ValidIndex(i.uvIndex, g.UVs().size()))) {
                    std::cerr << "obj: surface " << n << " of mesh '" << mesh->GetName()
                              << "' has an out of range index (v " << i.posIndex + 1 << " vt "
                              << i.uvIndex + 1 << " vn " << i.normIndex + 1 << ")" << std::endl;
                    return false;
                }
            }
            n++;
        }
    }
    return true;
}

} // namespace

std::unique_ptr<Geometry> LoadObj(std::istream &in) {
    auto g = std::make_unique<Geometry>();
    std::unique_ptr<Mesh> mesh;
    bool addedSurfaces = true; // a vertex after faces starts a new mesh
    int meshnum = 0;
    std::string nextname = "default0";

    std::string line;
    int lineno = 0;
    while (std::getline(in, line)) {
        lineno++;
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        std::string_view sv(line);

        if (sv.starts_with("v ")) {
            if (addedSurfaces) {
                if (mesh) {
                    g->AddMesh(std::move(mesh));
                }
                mesh = g->CreateMesh();
                addedSurfaces = false;
                meshnum++;
                mesh->SetName(nextname);
                nextname = "default" + std::to_string(meshnum);
            }
            Vertex v;
            ParseFloats(SplitSpaces(sv), 3, &v.x);
            g->AddVert(v);
        } else if (sv.starts_with("vn ")) {
            Vertex v;
            ParseFloats(SplitSpaces(sv), 3, &v.x);
            g->AddNormalVert(v);
        } else if (sv.starts_with("vt ")) {
            Vertex v;
            ParseFloats(SplitSpaces(sv), 2, &v.x);
            g->AddUVVert(v);
        } else if (sv.starts_with("f ")) {
            if (!mesh) {
                std::cerr << "obj: line " << lineno << ": face before any vertex" << std::endl;
                return nullptr;
            }
            addedSurfaces = true;

            std::vector<std::string_view> corners = SplitSpaces(sv);
            if (corners.size() < 4 || corners.size() > 5) {
                std::cerr << "obj: line " << lineno << ": skipping face with " << corners.size() - 1
                          << " vertices" << std::endl;
                continue;
            }

            Surface s;
            for (size_t c = 1; c < corners.size(); c++) {
                std::vector<std::string_view> fields = SplitSlashes(corners[c]);
                int pos = -1, uv = -1, norm = -1;
                if (!ParseIndex(fields[0], pos) ||
                    (fields.size() > 1 && !ParseIndex(fields[1], uv)) ||
                    (fields.size() > 2 && !ParseIndex(fields[2], norm))) {
                    std::cerr << "obj: line " << lineno << ": bad face vertex '" << corners[c]
                              << "'" << std::endl;
                    return nullptr;
                }
                s.AddIndex(pos, norm, uv);
            }
            mesh->AddSurface(std::move(s));
        } else if (sv.starts_with("#  ExternalName: ")) {
            nextname = std::string(sv.substr(17));

            // suffix a number until the name is unique
            std::string name = nextname;
            for (int n = 0;; n++) {
                bool taken = false;
                for (const auto &m : g->Meshes()) {
                    taken = taken || m->GetName() == name;
                }
                if (!taken) {
                    break;
                }
                name = nextname + std::to_string(n);
            }
            nextname = name;
        } else if (sv.starts_with("vp ") || sv.starts_with("mtllib") || sv.starts_with("usemtl") ||
                   sv.starts_with("g ") || sv.starts_with("s ") || sv.starts_with("l ") ||
                   sv.starts_with("p ") || sv[0] == '#') {
            // unsupported directives and comments
        } else {
            std::cerr << "obj: line " << lineno << ": unknown directive '" << sv << "'"
                      << std::endl;
        }
    }

    // a mesh only counts once it has faces
    if (mesh && addedSurfaces) {
        g->AddMesh(std::move(mesh));
    }

    if (!Validate(*g)) {
        return nullptr;
    }
    return g;
}

std::unique_ptr<Geometry> LoadObjFile(const std::string &path) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "obj: cannot open " << path << std::endl;
        return nullptr;
    }
    return LoadObj(in);
}
