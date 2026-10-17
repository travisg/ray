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
#include <cstdio>
#include <ctime>
#include <functional>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <Ray.h>
#include <RenderSurface.h>
#include <Scene.h>
#include <Tracer.h>

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
};

void Usage(FILE *fp, const char *argv0) {
    fprintf(fp,
            "usage: %s [options]\n"
            "  -x, --xres N    image width (default 800)\n"
            "  -y, --yres N    image height (default 600)\n"
            "  -c, --cpus N    worker threads (default: hardware threads)\n"
            "  -o, --out FILE  output .ray file (default out.ray)\n"
            "  -r, --random    trace tiles in random order\n"
            "  -h, --help      this message\n",
            argv0);
}

// Parses a positive integer; false on anything else.
bool ParsePositive(const char *s, int &out) {
    std::string_view sv(s);
    int v = 0;
    auto [end, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), v);
    if (ec != std::errc() || end != sv.data() + sv.size() || v < 1) {
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
        { "random", no_argument, nullptr, 'r' },
        { "help", no_argument, nullptr, 'h' },
        { nullptr, 0, nullptr, 0 },
    };

    exitCode = 1;
    int c;
    while ((c = getopt_long(argc, argv, "x:y:c:o:rh", kLongOptions, nullptr)) != -1) {
        switch (c) {
            case 'x':
            case 'y':
            case 'c': {
                int &target = c == 'x' ? opts.xres : c == 'y' ? opts.yres : opts.cpus;
                if (!ParsePositive(optarg, target)) {
                    fprintf(stderr, "%s: -%c needs a positive integer, got '%s'\n", argv[0], c,
                            optarg);
                    return false;
                }
                break;
            }
            case 'o':
                opts.outfile = optarg;
                break;
            case 'r':
                opts.random = true;
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

} // namespace

void TracerThread(const RenderSurface &rs, const Scene &scene, TraceMaster &master) {
    time_t start = time(nullptr);

    Tracer tracer(rs, scene, master);
    tracer.Trace();

    start = time(nullptr) - start;
    std::cout << "trace end, took " << start << " seconds" << std::endl;
}

int main(int argc, char *argv[]) {
    Options opts;
    int exitCode = 0;
    if (!ParseOptions(argc, argv, opts, exitCode)) {
        return exitCode;
    }

    printf("xres %d yres %d cpus %d\n", opts.xres, opts.yres, opts.cpus);
    std::cout << "outfile " << opts.outfile << std::endl;

    RenderSurfaceFile rs(opts.xres, opts.yres);
    if (rs.OpenOutFile(opts.outfile) < 0) {
        std::cerr << "error opening output file " << opts.outfile << std::endl;
        return 1;
    }

    Scene scene;

    std::unique_ptr<TraceMaster> master;
    if (opts.random) {
        master = std::make_unique<TraceMasterRandom>(rs);
    } else {
        master = std::make_unique<TraceMasterSimple>(rs);
    }

    std::vector<std::thread> threads;
    threads.reserve(opts.cpus);
    for (int i = 0; i < opts.cpus; i++) {
        threads.emplace_back(TracerThread, std::cref(rs), std::cref(scene), std::ref(*master));
    }

    // wait for the trace master to hand out and collect every tile
    master->WaitForDone();

    std::cout << "waiting for worker threads to shut down" << std::endl;
    for (auto &t : threads) {
        t.join();
    }

    std::cout << "shutting down" << std::endl;

    // rs closes the output file, which writes the EOF packet
    return 0;
}
