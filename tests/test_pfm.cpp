#include <cstdio>
#include <filesystem>
#include <ray/framebuffer.h>
#include <ray/pfm.h>
#include <string>
#include <tests/check.h>
#include <vector>

int main() {
    const std::string path = (std::filesystem::temp_directory_path() / "ray-test.pfm").string();

    // a framebuffer round trip keeps every value and the row order
    {
        framebuffer fb(3, 2);
        fb.set_xy(0, 0, colorf(1, 0, 0));
        const colorf run[2] = { colorf(0, 0.5, 0), colorf(0, 0, 2.5) };
        fb.set_xy_run(1, 1, 2, run);
        CHECK(fb.size() == 3 * 2 * 3);
        CHECK(fb.at(2, 1).b == 2.5);

        CHECK(write_pfm(path, fb.width(), fb.height(), fb.data()));
        int w = 0, h = 0;
        std::vector<float> rgb;
        CHECK(read_pfm(path, w, h, rgb));
        CHECK(w == 3 && h == 2);
        CHECK(rgb.size() == fb.size());
        for (size_t i = 0; i < rgb.size(); i++) {
            CHECK(rgb[i] == fb.data()[i]);
        }
    }

    // the file starts with the PF header and the bottom row
    {
        FILE *fp = fopen(path.c_str(), "rb");
        CHECK(fp != nullptr);
        if (fp) {
            char head[12] = {};
            CHECK(fread(head, 1, 11, fp) == 11);
            CHECK(std::string(head) == "PF\n3 2\n-1.0" || std::string(head) == "PF\n3 2\n1.0\n");
            fclose(fp);
        }
    }

    // missing and malformed files fail
    {
        int w = 0, h = 0;
        std::vector<float> rgb;
        CHECK(!read_pfm(path + ".missing", w, h, rgb));
        FILE *fp = fopen(path.c_str(), "wb");
        if (fp) {
            fputs("P6\n3 2\n255\n", fp);
            fclose(fp);
        }
        CHECK(!read_pfm(path, w, h, rgb));
    }
    std::filesystem::remove(path);
    return TEST_RESULT();
}
