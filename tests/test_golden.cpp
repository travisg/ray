// Compares a render against a reference .ray file channel by channel.
#include <RayFile.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace {

// Sub-visible (an 8-bit step is ~4e-3) yet far above FMA and libm noise.
constexpr float kTolerance = 1e-3f;

bool Load(const char *path, RayFile &f, std::vector<float> &pixels) {
    if (f.Open(path) < 0) {
        std::fprintf(stderr, "cannot open %s\n", path);
        return false;
    }
    pixels.assign(f.GetPixelCount() * 3, 0.0f);
    if (f.ReadIntoBuffer(pixels.data()) < 0) {
        std::fprintf(stderr, "cannot parse %s\n", path);
        return false;
    }
    return true;
}

} // namespace

int main(int argc, char **argv) {
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s <render.ray> <reference.ray>\n", argv[0]);
        return 2;
    }

    RayFile a, b;
    std::vector<float> pa, pb;
    if (!Load(argv[1], a, pa) || !Load(argv[2], b, pb)) {
        return 1;
    }
    if (a.Width() != b.Width() || a.Height() != b.Height()) {
        std::fprintf(stderr, "size mismatch: %ux%u vs %ux%u\n", a.Width(), a.Height(), b.Width(),
                     b.Height());
        return 1;
    }

    size_t bad = 0;
    float maxDiff = 0;
    for (size_t i = 0; i < pa.size(); i++) {
        float d = std::fabs(pa[i] - pb[i]);
        if (!(d <= kTolerance)) {
            bad++;
        }
        maxDiff = std::max(maxDiff, d);
    }
    if (bad) {
        std::fprintf(stderr, "%zu of %zu channels differ by more than %g (max diff %g)\n", bad,
                     pa.size(), kTolerance, maxDiff);
        return 1;
    }
    std::printf("%zu channels within %g (max diff %g)\n", pa.size(), kTolerance, maxDiff);
    return 0;
}
