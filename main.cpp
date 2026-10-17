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
#include <getopt.h>
#include <iostream>
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

int TracerThread(RenderSurface *rs, Scene *scene, TraceMaster *trace_master) {
    time_t start = time(nullptr);

    Tracer *tracer = new Tracer(*rs, *scene, *trace_master);
    tracer->Trace();

    start = time(nullptr) - start;
    std::cout << "trace end, took " << start << " seconds" << std::endl;

    delete tracer;

    return 0;
}

int main(int argc, char *argv[]) {
    Options opts;
    int exitCode = 0;
    if (!ParseOptions(argc, argv, opts, exitCode)) {
        return exitCode;
    }

    printf("xres %d yres %d cpus %d\n", opts.xres, opts.yres, opts.cpus);
    std::cout << "outfile " << opts.outfile << std::endl;

    auto *rs = new RenderSurfaceFile(opts.xres, opts.yres);

    if (rs->OpenOutFile(opts.outfile) < 0) {
        std::cerr << "error opening output file " << opts.outfile << std::endl;
        return 1;
    }

    /* create the scene */
    auto *scene = new Scene();

    /* create a tracemaster */
    TraceMaster *trace_master;
    if (opts.random) {
        trace_master = new TraceMasterRandom(*rs);
    } else {
        trace_master = new TraceMasterSimple(*rs);
    }

    std::vector<std::thread *> threads;
    for (int i = 0; i < opts.cpus; i++) {
        auto *t = new std::thread(TracerThread, rs, scene, trace_master);
        threads.push_back(t);
    }

    // wait for the trace master to quit handing out work units
    trace_master->WaitForDone();

    // wait for the worker threads to complete
    std::cout << "waiting for worker threads to shut down" << std::endl;
    while (!threads.empty()) {
        auto *t = threads.back();
        threads.pop_back();
        t->join();
        delete t;
    }

    std::cout << "shutting down" << std::endl;

    delete trace_master;
    delete scene;
    delete rs;

    return 0;
}
