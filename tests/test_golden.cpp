// Compares a render against a reference .ray file channel by channel.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ray/ray_file.h>
#include <vector>

namespace {

// Sub-visible (an 8-bit step is ~4e-3) yet far above FMA and libm noise.
constexpr float kTolerance = 1e-3f;

bool load(const char *path, ray_file &f, std::vector<float> &pixels) {
    if (f.open(path) < 0) {
        std::fprintf(stderr, "cannot open %s\n", path);
        return false;
    }
    if (f.read_into_buffer(pixels) < 0) {
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

    ray_file a, b;
    std::vector<float> pa, pb;
    if (!load(argv[1], a, pa) || !load(argv[2], b, pb)) {
        return 1;
    }
    if (a.width() != b.width() || a.height() != b.height()) {
        std::fprintf(stderr, "size mismatch: %ux%u vs %ux%u\n", a.width(), a.height(), b.width(),
                     b.height());
        return 1;
    }

    size_t bad = 0;
    float max_diff = 0;
    for (size_t i = 0; i < pa.size(); i++) {
        float d = std::fabs(pa[i] - pb[i]);
        if (!(d <= kTolerance)) {
            bad++;
        }
        max_diff = std::max(max_diff, d);
    }
    if (bad) {
        std::fprintf(stderr, "%zu of %zu channels differ by more than %g (max diff %g)\n", bad,
                     pa.size(), kTolerance, max_diff);
        return 1;
    }
    std::printf("%zu channels within %g (max diff %g)\n", pa.size(), kTolerance, max_diff);
    return 0;
}
