// Compares a render against a reference .pfm channel by channel.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ray/pfm.h>
#include <vector>

namespace {

// Sub-visible (an 8-bit step is ~4e-3) yet far above FMA and libm noise.
constexpr float kTolerance = 1e-3f;

bool load(const char *path, int &w, int &h, std::vector<float> &pixels) {
    if (!read_pfm(path, w, h, pixels)) {
        std::fprintf(stderr, "cannot read %s\n", path);
        return false;
    }
    return true;
}

} // namespace

int main(int argc, char **argv) {
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s <render.pfm> <reference.pfm>\n", argv[0]);
        return 2;
    }

    int wa = 0, ha = 0, wb = 0, hb = 0;
    std::vector<float> pa, pb;
    if (!load(argv[1], wa, ha, pa) || !load(argv[2], wb, hb, pb)) {
        return 1;
    }
    if (wa != wb || ha != hb) {
        std::fprintf(stderr, "size mismatch: %dx%d vs %dx%d\n", wa, ha, wb, hb);
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
