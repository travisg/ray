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
#include <atomic>
#include <charconv>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <ray/framebuffer.h>
#include <ray/pfm.h>
#include <ray/png.h>
#include <ray/ray.h>
#include <ray/sampling.h>
#include <ray/scene.h>
#include <ray/scene_library.h>
#include <ray/tev_display.h>
#include <ray/trace_master.h>
#include <ray/tracer.h>

namespace {

int default_cpus() {
    unsigned n = std::thread::hardware_concurrency();
    return n == 0 ? 1 : static_cast<int>(n);
}

struct options {
    int xres = 800;
    int yres = 600;
    int cpus = default_cpus();
    std::string outfile = "out.png";
    bool random = false;
    uint32_t seed = 1;
    std::string scene = std::string(kDefaultSceneName);
    bool list_scenes = false;
    std::string display; // HOST[:PORT] of a tev viewer, empty for none
    int gi_samples = 0;
    int bounces = 1;
    int passes = 1; // 0: until interrupted
};

enum { opt_seed = 256, opt_list_scenes, opt_display, opt_gi, opt_bounces, opt_passes };

void usage(FILE *fp, const char *argv0) {
    fprintf(fp,
            "usage: %s [options]\n"
            "  -x, --xres N      image width (default 800)\n"
            "  -y, --yres N      image height (default 600)\n"
            "  -c, --cpus N      worker threads (default: hardware threads)\n"
            "  -o, --out FILE    output, by extension: .png (8-bit sRGB) or .pfm (linear float),\n"
            "                    written when the render ends (default out.png)\n"
            "  -s, --scene NAME  scene to render (default: default)\n"
            "      --list-scenes\n"
            "  -r, --random      trace tiles in random order\n"
            "      --seed N      seed for --random and the random scenes (default 1)\n"
            "      --gi N        indirect light: N hemisphere rays per hit, rounded up to a\n"
            "                    grid of strata (default 0, off)\n"
            "      --bounces N   bounces of indirect light (default 1)\n"
            "      --passes N    render N passes and average them, each with fresh random\n"
            "                    numbers and the ray jittered within its pixel (default 1;\n"
            "                    0 keeps going until Ctrl-C, which writes the image so far)\n"
            "      --display HOST[:PORT]\n"
            "                    push tiles to a tev viewer as they finish (port 14158)\n"
            "  -h, --help        this message\n",
            argv0);
}

// Parses an integer in [min, max]; false on anything else.
template <typename T> bool parse_int(const char *s, T min, T max, T &out) {
    std::string_view sv(s);
    T v = 0;
    auto [end, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), v);
    if (ec != std::errc() || end != sv.data() + sv.size() || v < min || v > max) {
        return false;
    }
    out = v;
    return true;
}

// Returns true to render; otherwise exit_code holds the status to exit with.
bool parse_options(int argc, char **argv, options &opts, int &exit_code) {
    static const option kLongOptions[] = {
        { "xres", required_argument, nullptr, 'x' },
        { "yres", required_argument, nullptr, 'y' },
        { "cpus", required_argument, nullptr, 'c' },
        { "out", required_argument, nullptr, 'o' },
        { "scene", required_argument, nullptr, 's' },
        { "list-scenes", no_argument, nullptr, opt_list_scenes },
        { "random", no_argument, nullptr, 'r' },
        { "seed", required_argument, nullptr, opt_seed },
        { "display", required_argument, nullptr, opt_display },
        { "gi", required_argument, nullptr, opt_gi },
        { "bounces", required_argument, nullptr, opt_bounces },
        { "passes", required_argument, nullptr, opt_passes },
        { "help", no_argument, nullptr, 'h' },
        { nullptr, 0, nullptr, 0 },
    };

    exit_code = 1;
    int c;
    while ((c = getopt_long(argc, argv, "x:y:c:o:s:rh", kLongOptions, nullptr)) != -1) {
        switch (c) {
            case 'x':
            case 'y':
            case 'c': {
                int &target = c == 'x' ? opts.xres : c == 'y' ? opts.yres : opts.cpus;
                if (!parse_int(optarg, 1, INT32_MAX, target)) {
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
            case opt_list_scenes:
                opts.list_scenes = true;
                break;
            case 'r':
                opts.random = true;
                break;
            case opt_seed:
                if (!parse_int(optarg, uint32_t{ 0 }, UINT32_MAX, opts.seed)) {
                    fprintf(stderr, "%s: --seed needs an unsigned integer, got '%s'\n", argv[0],
                            optarg);
                    return false;
                }
                break;
            case opt_display:
                opts.display = optarg;
                break;
            case opt_gi:
                if (!parse_int(optarg, 0, 1 << 20, opts.gi_samples)) {
                    fprintf(stderr, "%s: --gi needs a non-negative integer, got '%s'\n", argv[0],
                            optarg);
                    return false;
                }
                break;
            case opt_bounces:
                if (!parse_int(optarg, 1, 15, opts.bounces)) {
                    fprintf(stderr, "%s: --bounces needs an integer from 1 to 15, got '%s'\n",
                            argv[0], optarg);
                    return false;
                }
                break;
            case opt_passes:
                if (!parse_int(optarg, 0, INT32_MAX, opts.passes)) {
                    fprintf(stderr, "%s: --passes needs a non-negative integer, got '%s'\n",
                            argv[0], optarg);
                    return false;
                }
                break;
            case 'h':
                usage(stdout, argv[0]);
                exit_code = 0;
                return false;
            default:
                usage(stderr, argv[0]);
                return false;
        }
    }
    if (optind != argc) {
        fprintf(stderr, "%s: unexpected argument '%s'\n", argv[0], argv[optind]);
        return false;
    }
    return true;
}

// Set by the first Ctrl-C; the render ends after the current pass and writes its image. The
// handler restores the default so a second Ctrl-C kills the process as usual.
volatile std::sig_atomic_t g_interrupted = 0;

void on_interrupt(int) {
    g_interrupted = 1;
    signal(SIGINT, SIG_DFL);
}

// HOST[:PORT]; tev listens on 14158 unless told otherwise
std::pair<std::string, std::string> split_host_port(const std::string &spec) {
    const size_t colon = spec.rfind(':');
    if (colon == std::string::npos) {
        return { spec, "14158" };
    }
    return { spec.substr(0, colon), spec.substr(colon + 1) };
}

constexpr auto kDisplayRefresh = std::chrono::milliseconds(100);
constexpr auto kDisplayRetry = std::chrono::seconds(1);

// Pushes what changed to the viewer every kDisplayRefresh. A viewer that appears late, or comes
// back after dropping the connection, gets the whole image so far; one last pass after stop
// flushes the final tiles.
void display_thread(std::atomic<bool> &stop, framebuffer &fb, tev_display &display) {
    std::vector<float> pixels;
    auto last_try = std::chrono::steady_clock::now();
    for (;;) {
        const bool last = stop.load();
        if (!display.connected()) {
            const auto now = std::chrono::steady_clock::now();
            if (now - last_try >= kDisplayRetry) {
                last_try = now;
                if (display.connect()) {
                    fb.mark_all_dirty();
                }
            }
        }
        if (display.connected()) {
            if (std::optional<framebuffer::rect> r = fb.take_dirty(pixels)) {
                display.update(r->x, r->y, r->w, r->h, pixels.data(), r->w);
            }
        }
        if (last) {
            return;
        }
        std::this_thread::sleep_for(kDisplayRefresh);
    }
}

// Runs one tracer until the tiles run out and adds its busy time to total_us.
void tracer_thread(const framebuffer &fb, const scene &s, trace_master &master,
                   const render_settings &settings, std::atomic<int64_t> &total_us) {
    const auto start = std::chrono::steady_clock::now();
    tracer t(fb, s, master, settings);
    t.trace();
    total_us += std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::steady_clock::now() - start)
                    .count();
}

} // namespace

int main(int argc, char *argv[]) {
    options opts;
    int exit_code = 0;
    if (!parse_options(argc, argv, opts, exit_code)) {
        return exit_code;
    }

    if (opts.list_scenes) {
        for (const scene_info &info : list_scenes()) {
            printf("%-12.*s %.*s\n", (int)info.name.size(), info.name.data(),
                   (int)info.description.size(), info.description.data());
        }
        return 0;
    }

    // the extension picks the output, written from the framebuffer once the render ends
    const bool to_png = opts.outfile.ends_with(".png");
    if (!to_png && !opts.outfile.ends_with(".pfm")) {
        std::cerr << "output file " << opts.outfile << " must end in .png or .pfm\n";
        return 1;
    }

    printf("rendering %s at %dx%d on %d threads to %s", opts.scene.c_str(), opts.xres, opts.yres,
           opts.cpus, opts.outfile.c_str());
    if (opts.gi_samples > 0) {
        const strata grid = stratify(opts.gi_samples);
        printf(", %d indirect rays per hit (%dx%d strata), %d bounce%s", grid.count(), grid.x,
               grid.y, opts.bounces, opts.bounces == 1 ? "" : "s");
    }
    if (opts.passes == 0) {
        printf(", passes until Ctrl-C");
    } else if (opts.passes > 1) {
        printf(", %d passes", opts.passes);
    }
    printf("\n");

    std::mt19937 rng(opts.seed);
    std::unique_ptr<scene> s = build_scene(opts.scene, rng);
    if (!s) {
        return 1;
    }

    framebuffer fb(opts.xres, opts.yres);

    std::unique_ptr<tev_display> display;
    if (!opts.display.empty()) {
        const auto [host, port] = split_host_port(opts.display);
        // the image is named after the output file, so re-rendering replaces it in the viewer
        display = std::make_unique<tev_display>(
            host, port, std::filesystem::path(opts.outfile).filename().string(), opts.xres,
            opts.yres);
        if (!display->connect()) {
            std::cerr << "display: nothing listening on " << host << ":" << port
                      << ", will keep trying\n";
        }
    }

    std::atomic<bool> stop_display{ false };
    std::thread display_worker;
    if (display) {
        display_worker =
            std::thread(display_thread, std::ref(stop_display), std::ref(fb), std::ref(*display));
    }

    render_settings settings;
    settings.gi_samples = opts.gi_samples;
    settings.bounces = opts.bounces;
    settings.seed = opts.seed;
    settings.jitter = opts.passes != 1;

    signal(SIGINT, on_interrupt);

    // one trace_master per pass; the framebuffer keeps the running mean of the passes
    const auto start = std::chrono::steady_clock::now();
    std::atomic<int64_t> thread_us{ 0 };
    int passes = 0;
    for (; (opts.passes == 0 || passes < opts.passes) && !g_interrupted; passes++) {
        settings.pass = passes;
        trace_master master(fb, opts.random ? tile_order::random : tile_order::sequential,
                            opts.seed + static_cast<uint32_t>(passes), passes);
        std::vector<std::thread> threads;
        threads.reserve(opts.cpus);
        for (int i = 0; i < opts.cpus; i++) {
            threads.emplace_back(tracer_thread, std::cref(fb), std::cref(*s), std::ref(master),
                                 std::cref(settings), std::ref(thread_us));
        }
        // wait for the trace master to hand out and collect every tile
        master.wait_for_done();
        for (auto &t : threads) {
            t.join();
        }
    }
    const double wall =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    if (opts.passes == 1) {
        printf("rendered in %.3f s wall, %.1f s across %d threads\n", wall, thread_us / 1e6,
               opts.cpus);
    } else {
        printf("%s %d pass%s in %.3f s wall, %.1f s across %d threads\n",
               g_interrupted ? "interrupted after" : "rendered", passes, passes == 1 ? "" : "es",
               wall, thread_us / 1e6, opts.cpus);
    }

    if (display_worker.joinable()) {
        stop_display = true;
        display_worker.join();
    }

    const bool ok = to_png ? write_png(opts.outfile, fb.width(), fb.height(), fb.data())
                           : write_pfm(opts.outfile, fb.width(), fb.height(), fb.data());
    if (!ok) {
        std::cerr << "error writing " << opts.outfile << "\n";
        return 1;
    }
    return 0;
}
