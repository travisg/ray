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
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <functional>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <ray.h>
#include <render_surface.h>
#include <scene.h>
#include <scene_library.h>
#include <trace_master.h>
#include <tracer.h>

namespace {

int DefaultCpus() {
    unsigned n = std::thread::hardware_concurrency();
    return n == 0 ? 1 : static_cast<int>(n);
}

struct Options {
    int xres = 800;
    int yres = 600;
    int cpus = DefaultCpus();
    std::string outfile = "out.ray";
    bool random = false;
    uint32_t seed = 1;
    std::string scene = std::string(kDefaultSceneName);
    bool listScenes = false;
};

enum { kOptSeed = 256, kOptListScenes };

void Usage(FILE *fp, const char *argv0) {
    fprintf(fp,
            "usage: %s [options]\n"
            "  -x, --xres N      image width (default 800)\n"
            "  -y, --yres N      image height (default 600)\n"
            "  -c, --cpus N      worker threads (default: hardware threads)\n"
            "  -o, --out FILE    output .ray file (default out.ray)\n"
            "  -s, --scene NAME  scene to render (default: default)\n"
            "      --list-scenes\n"
            "  -r, --random      trace tiles in random order\n"
            "      --seed N      seed for --random and the random scenes (default 1)\n"
            "  -h, --help        this message\n",
            argv0);
}

// Parses an integer in [min, max]; false on anything else.
template <typename T> bool ParseInt(const char *s, T min, T max, T &out) {
    std::string_view sv(s);
    T v = 0;
    auto [end, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), v);
    if (ec != std::errc() || end != sv.data() + sv.size() || v < min || v > max) {
        return false;
    }
    out = v;
    return true;
}

// Returns true to render; otherwise exitCode holds the status to exit with.
bool ParseOptions(int argc, char **argv, Options &opts, int &exitCode) {
    static const option kLongOptions[] = {
        { "xres", required_argument, nullptr, 'x' },
        { "yres", required_argument, nullptr, 'y' },
        { "cpus", required_argument, nullptr, 'c' },
        { "out", required_argument, nullptr, 'o' },
        { "scene", required_argument, nullptr, 's' },
        { "list-scenes", no_argument, nullptr, kOptListScenes },
        { "random", no_argument, nullptr, 'r' },
        { "seed", required_argument, nullptr, kOptSeed },
        { "help", no_argument, nullptr, 'h' },
        { nullptr, 0, nullptr, 0 },
    };

    exitCode = 1;
    int c;
    while ((c = getopt_long(argc, argv, "x:y:c:o:s:rh", kLongOptions, nullptr)) != -1) {
        switch (c) {
            case 'x':
            case 'y':
            case 'c': {
                int &target = c == 'x' ? opts.xres : c == 'y' ? opts.yres : opts.cpus;
                if (!ParseInt(optarg, 1, INT32_MAX, target)) {
                    fprintf(stderr, "%s: -%c needs a positive integer, got '%s'\n", argv[0], c,
                            optarg);
                    return false;
                }
                break;
            }
            case 'o':
                opts.outfile = optarg;
                break;
            case 's':
                opts.scene = optarg;
                break;
            case kOptListScenes:
                opts.listScenes = true;
                break;
            case 'r':
                opts.random = true;
                break;
            case kOptSeed:
                if (!ParseInt(optarg, uint32_t{ 0 }, UINT32_MAX, opts.seed)) {
                    fprintf(stderr, "%s: --seed needs an unsigned integer, got '%s'\n", argv[0],
                            optarg);
                    return false;
                }
                break;
            case 'h':
                Usage(stdout, argv[0]);
                exitCode = 0;
                return false;
            default:
                Usage(stderr, argv[0]);
                return false;
        }
    }
    if (optind != argc) {
        fprintf(stderr, "%s: unexpected argument '%s'\n", argv[0], argv[optind]);
        return false;
    }
    return true;
}

void TracerThread(const RenderSurface &rs, const Scene &scene, TraceMaster &master) {
    time_t start = time(nullptr);

    Tracer tracer(rs, scene, master);
    tracer.Trace();

    start = time(nullptr) - start;
    std::cout << "trace end, took " << start << " seconds" << std::endl;
}

} // namespace

int main(int argc, char *argv[]) {
    Options opts;
    int exitCode = 0;
    if (!ParseOptions(argc, argv, opts, exitCode)) {
        return exitCode;
    }

    if (opts.listScenes) {
        for (const SceneInfo &info : ListScenes()) {
            printf("%-12.*s %.*s\n", (int)info.name.size(), info.name.data(),
                   (int)info.description.size(), info.description.data());
        }
        return 0;
    }

    printf("xres %d yres %d cpus %d scene %s\n", opts.xres, opts.yres, opts.cpus,
           opts.scene.c_str());
    std::cout << "outfile " << opts.outfile << std::endl;

    std::mt19937 rng(opts.seed);
    std::unique_ptr<Scene> scene = BuildScene(opts.scene, rng);
    if (!scene) {
        return 1;
    }

    RenderSurfaceFile rs(opts.xres, opts.yres);
    if (rs.OpenOutFile(opts.outfile) < 0) {
        std::cerr << "error opening output file " << opts.outfile << std::endl;
        return 1;
    }

    TraceMaster master(rs, opts.random ? TileOrder::Random : TileOrder::Sequential, opts.seed);

    std::vector<std::thread> threads;
    threads.reserve(opts.cpus);
    for (int i = 0; i < opts.cpus; i++) {
        threads.emplace_back(TracerThread, std::cref(rs), std::cref(*scene), std::ref(master));
    }

    // wait for the trace master to hand out and collect every tile
    master.WaitForDone();

    std::cout << "waiting for worker threads to shut down" << std::endl;
    for (auto &t : threads) {
        t.join();
    }

    std::cout << "shutting down" << std::endl;

    // rs closes the output file, which writes the EOF packet
    return 0;
}
