#include "check.h"
#include <ray_file.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

int main() {
    const fs::path path = fs::temp_directory_path() / "ray-test-roundtrip.ray";

    // write: one pixel, a colorf run and a raw float run on a 4x3 image
    {
        RayFile f;
        CHECK(f.OpenWrite(path.string(), 4, 3) == 0);
        f.SetXY(0, 0, colorf(0.1, 0.2, 0.3));
        const colorf run[3] = { colorf(1, 0, 0), colorf(0, 1, 0), colorf(0, 0, 1) };
        f.SetXYRun(1, 0, 3, run);
        const float raw[4 * 3] = { 0.5f, 0.5f, 0.5f, 1.5f, 0, 0, 0, 1.5f, 0, 0, 0, 1.5f };
        f.SetXYRun(0, 1, 4, raw);
        f.Close();
    }

    // read it back
    {
        RayFile f;
        CHECK(f.Open(path.string()) == 0);
        CHECK(f.Width() == 4);
        CHECK(f.Height() == 3);
        CHECK(f.GetPixelCount() == 12);

        std::vector<float> buf;
        CHECK(f.ReadIntoBuffer(buf) == 0);
        CHECK(buf.size() == 12 * 3);
        auto px = [&](int x, int y, int ch) { return buf[(y * 4 + x) * 3 + ch]; };
        CHECK_NEAR(px(0, 0, 0), 0.1f, 1e-6);
        CHECK_NEAR(px(0, 0, 1), 0.2f, 1e-6);
        CHECK_NEAR(px(0, 0, 2), 0.3f, 1e-6);
        CHECK(px(1, 0, 0) == 1.0f && px(1, 0, 1) == 0.0f);
        CHECK(px(2, 0, 1) == 1.0f);
        CHECK(px(3, 0, 2) == 1.0f);
        CHECK(px(0, 1, 0) == 0.5f);
        CHECK(px(1, 1, 0) == 1.5f);
        CHECK(px(3, 1, 2) == 1.5f);
        // row 2 was never written
        CHECK(px(0, 2, 0) == 0.0f);
        f.Close();
    }

    // missing file and bad magic both fail to open
    {
        RayFile f;
        CHECK(f.Open((fs::temp_directory_path() / "ray-test-missing.ray").string()) < 0);
    }
    {
        const fs::path bad = fs::temp_directory_path() / "ray-test-badmagic.ray";
        FILE *fp = std::fopen(bad.string().c_str(), "wb");
        CHECK(fp != nullptr);
        if (fp) {
            const uint32_t header[3] = { 0x12345678, 4, 3 };
            std::fwrite(header, sizeof(header), 1, fp);
            std::fclose(fp);
        }
        RayFile f;
        CHECK(f.Open(bad.string()) < 0);
        fs::remove(bad);
    }

    // the stream reader stops at an incomplete packet and resumes when the rest arrives
    {
        const fs::path grow = fs::temp_directory_path() / "ray-test-stream.ray";
        const uint32_t header[3] = { kRayHeaderMagic, 4, 1 };
        const uint32_t run[4] = { TYPE_PIXEL_RUN, 1, 0, 2 };
        const float rgb[6] = { 1, 2, 3, 4, 5, 6 };
        const uint32_t eof = TYPE_EOF;

        FILE *w = std::fopen(grow.string().c_str(), "wb");
        CHECK(w != nullptr);
        FILE *r = std::fopen(grow.string().c_str(), "rb");
        CHECK(r != nullptr);
        if (w && r) {
            RayStreamReader reader(r);
            CHECK(reader.ReadHeader() == RayStreamReader::Status::NeedData);

            std::fwrite(header, sizeof(header), 1, w);
            std::fwrite(run, sizeof(run), 1, w);
            std::fwrite(rgb, sizeof(float), 3, w); // only the first pixel of the run
            std::fflush(w);
            CHECK(reader.ReadHeader() == RayStreamReader::Status::Ok);
            CHECK(reader.Width() == 4 && reader.Height() == 1);
            int runs = 0;
            RayRunCallback count = [&](uint32_t, uint32_t, uint32_t, const float *) { runs++; };
            CHECK(reader.Pump(count) == RayStreamReader::Status::NeedData);
            CHECK(runs == 0);

            std::fwrite(rgb + 3, sizeof(float), 3, w);
            std::fwrite(&eof, sizeof(eof), 1, w);
            std::fflush(w);
            uint32_t gotx = 0, gotcount = 0;
            float got[6] = {};
            RayRunCallback grab = [&](uint32_t x, uint32_t, uint32_t n, const float *p) {
                runs++;
                gotx = x;
                gotcount = n;
                std::copy(p, p + 6, got);
            };
            CHECK(reader.Pump(grab) == RayStreamReader::Status::Eof);
            CHECK(runs == 1 && gotx == 1 && gotcount == 2);
            CHECK(got[0] == 1 && got[5] == 6);
        }
        if (w) {
            std::fclose(w);
        }
        if (r) {
            std::fclose(r);
        }
        fs::remove(grow);
    }

    // a run that overflows the image is an error
    {
        const fs::path bad = fs::temp_directory_path() / "ray-test-overflow.ray";
        FILE *w = std::fopen(bad.string().c_str(), "wb");
        CHECK(w != nullptr);
        if (w) {
            const uint32_t header[3] = { kRayHeaderMagic, 4, 1 };
            const uint32_t run[4] = { TYPE_PIXEL_RUN, 3, 0, 2 };
            std::fwrite(header, sizeof(header), 1, w);
            std::fwrite(run, sizeof(run), 1, w);
            std::fclose(w);
        }
        RayFile f;
        CHECK(f.Open(bad.string()) == 0);
        std::vector<float> buf;
        CHECK(f.ReadIntoBuffer(buf) < 0);
        f.Close();
        fs::remove(bad);
    }

    fs::remove(path);
    return TEST_RESULT();
}
