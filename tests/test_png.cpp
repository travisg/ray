#include <cstdio>
#include <cstring>
#include <filesystem>
#include <ray/png.h>
#include <string>
#include <tests/check.h>

int main() {
    // the sRGB curve: black, white, clamping, and the mid-grey everyone knows
    CHECK(linear_to_srgb8(0.0f) == 0);
    CHECK(linear_to_srgb8(1.0f) == 255);
    CHECK(linear_to_srgb8(2.0f) == 255);
    CHECK(linear_to_srgb8(-1.0f) == 0);
    CHECK(linear_to_srgb8(0.5f) == 188);
    CHECK(linear_to_srgb8(0.0031308f) == 10);

    // a 2x1 image lands on disk as a PNG
    const std::string path = (std::filesystem::temp_directory_path() / "ray-test.png").string();
    const float rgb[6] = { 1, 0, 0, 0, 0, 1 };
    CHECK(write_png(path, 2, 1, rgb));
    FILE *fp = fopen(path.c_str(), "rb");
    CHECK(fp != nullptr);
    if (fp) {
        unsigned char sig[8] = {};
        CHECK(fread(sig, 1, 8, fp) == 8);
        CHECK(memcmp(sig, "\x89PNG\r\n\x1a\n", 8) == 0);
        fclose(fp);
    }
    std::filesystem::remove(path);

    // an unwritable path fails
    CHECK(!write_png("/nonexistent-dir/ray-test.png", 2, 1, rgb));
    return TEST_RESULT();
}
