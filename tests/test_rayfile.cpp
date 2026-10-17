#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <ray/ray_file.h>
#include <tests/check.h>
#include <vector>

namespace fs = std::filesystem;

int main() {
    const fs::path path = fs::temp_directory_path() / "ray-test-roundtrip.ray";

    // write: one pixel, a colorf run and a raw float run on a 4x3 image
    {
        ray_file f;
        CHECK(f.open_write(path.string(), 4, 3) == 0);
        f.set_xy(0, 0, colorf(0.1, 0.2, 0.3));
        const colorf run[3] = { colorf(1, 0, 0), colorf(0, 1, 0), colorf(0, 0, 1) };
        f.set_xy_run(1, 0, 3, run);
        const float raw[4 * 3] = { 0.5f, 0.5f, 0.5f, 1.5f, 0, 0, 0, 1.5f, 0, 0, 0, 1.5f };
        f.set_xy_run(0, 1, 4, raw);
        f.close();
    }

    // read it back
    {
        ray_file f;
        CHECK(f.open(path.string()) == 0);
        CHECK(f.width() == 4);
        CHECK(f.height() == 3);
        CHECK(f.pixel_count() == 12);

        std::vector<float> buf;
        CHECK(f.read_into_buffer(buf) == 0);
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
        f.close();
    }

    // missing file and bad magic both fail to open
    {
        ray_file f;
        CHECK(f.open((fs::temp_directory_path() / "ray-test-missing.ray").string()) < 0);
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
        ray_file f;
        CHECK(f.open(bad.string()) < 0);
        fs::remove(bad);
    }

    // the stream reader stops at an incomplete packet and resumes when the rest arrives
    {
        const fs::path grow = fs::temp_directory_path() / "ray-test-stream.ray";
        const uint32_t header[3] = { kRayHeaderMagic, 4, 1 };
        const uint32_t run[4] = { static_cast<uint32_t>(ray_data_type::pixel_run), 1, 0, 2 };
        const float rgb[6] = { 1, 2, 3, 4, 5, 6 };
        const uint32_t eof = static_cast<uint32_t>(ray_data_type::eof);

        FILE *w = std::fopen(grow.string().c_str(), "wb");
        CHECK(w != nullptr);
        FILE *r = std::fopen(grow.string().c_str(), "rb");
        CHECK(r != nullptr);
        if (w && r) {
            ray_stream_reader reader(r);
            CHECK(reader.read_header() == ray_stream_reader::status::need_data);

            std::fwrite(header, sizeof(header), 1, w);
            std::fwrite(run, sizeof(run), 1, w);
            std::fwrite(rgb, sizeof(float), 3, w); // only the first pixel of the run
            std::fflush(w);
            CHECK(reader.read_header() == ray_stream_reader::status::ok);
            CHECK(reader.width() == 4 && reader.height() == 1);
            int runs = 0;
            ray_run_callback count = [&](uint32_t, uint32_t, uint32_t, const float *) { runs++; };
            CHECK(reader.pump(count) == ray_stream_reader::status::need_data);
            CHECK(runs == 0);

            std::fwrite(rgb + 3, sizeof(float), 3, w);
            std::fwrite(&eof, sizeof(eof), 1, w);
            std::fflush(w);
            uint32_t gotx = 0, gotcount = 0;
            float got[6] = {};
            ray_run_callback grab = [&](uint32_t x, uint32_t, uint32_t n, const float *p) {
                runs++;
                gotx = x;
                gotcount = n;
                std::copy(p, p + 6, got);
            };
            CHECK(reader.pump(grab) == ray_stream_reader::status::eof);
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
            const uint32_t run[4] = { static_cast<uint32_t>(ray_data_type::pixel_run), 3, 0, 2 };
            std::fwrite(header, sizeof(header), 1, w);
            std::fwrite(run, sizeof(run), 1, w);
            std::fclose(w);
        }
        ray_file f;
        CHECK(f.open(bad.string()) == 0);
        std::vector<float> buf;
        CHECK(f.read_into_buffer(buf) < 0);
        f.close();
        fs::remove(bad);
    }

    fs::remove(path);
    return TEST_RESULT();
}
