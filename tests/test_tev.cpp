#include <cstring>
#include <netinet/in.h>
#include <ray/framebuffer.h>
#include <ray/tev_display.h>
#include <string>
#include <sys/socket.h>
#include <tests/check.h>
#include <unistd.h>
#include <vector>

namespace {

// A listening socket on an ephemeral localhost port, standing in for tev.
int listen_local(uint16_t &port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (fd < 0 || bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0 ||
        listen(fd, 1) != 0) {
        return -1;
    }
    socklen_t len = sizeof(addr);
    getsockname(fd, reinterpret_cast<sockaddr *>(&addr), &len);
    port = ntohs(addr.sin_port);
    return fd;
}

// Reads one length-prefixed packet, length included in the returned bytes.
std::vector<uint8_t> read_packet(int fd) {
    std::vector<uint8_t> buf(4);
    if (recv(fd, buf.data(), 4, MSG_WAITALL) != 4) {
        return {};
    }
    uint32_t n = 0;
    memcpy(&n, buf.data(), 4);
    buf.resize(n);
    if (recv(fd, buf.data() + 4, n - 4, MSG_WAITALL) != static_cast<ssize_t>(n - 4)) {
        return {};
    }
    return buf;
}

// Little-endian field readers over a packet.
struct cursor {
    const std::vector<uint8_t> &buf;
    size_t pos;
    uint8_t u8() { return buf[pos++]; }
    uint32_t u32() {
        uint32_t v = 0;
        memcpy(&v, &buf[pos], 4);
        pos += 4;
        return v;
    }
    float f32() {
        float v = 0;
        memcpy(&v, &buf[pos], 4);
        pos += 4;
        return v;
    }
    std::string str() {
        std::string s(reinterpret_cast<const char *>(&buf[pos]));
        pos += s.size() + 1;
        return s;
    }
};

} // namespace

int main() {
    uint16_t port = 0;
    const int server = listen_local(port);
    CHECK(server >= 0);

    // nothing listening on a closed port is a quiet failure
    {
        tev_display d("127.0.0.1", "1", "img", 4, 3);
        CHECK(!d.connect());
        CHECK(!d.connected());
        CHECK(!d.update(0, 0, 1, 1, nullptr, 1));
    }

    tev_display d("127.0.0.1", std::to_string(port), "img", 4, 3);
    CHECK(d.connect());
    CHECK(d.connected());
    const int conn = accept(server, nullptr, nullptr);
    CHECK(conn >= 0);

    // CreateImage: grab focus, name, size, three channels
    {
        std::vector<uint8_t> p = read_packet(conn);
        cursor c{ p, 0 };
        CHECK(c.u32() == p.size());
        CHECK(c.u8() == 4);
        CHECK(c.u8() == 1);
        CHECK(c.str() == "img");
        CHECK(c.u32() == 4);
        CHECK(c.u32() == 3);
        CHECK(c.u32() == 3);
        CHECK(c.str() == "R");
        CHECK(c.str() == "G");
        CHECK(c.str() == "B");
        CHECK(c.pos == p.size());
    }

    // UpdateImageV2 of a 2x2 rectangle at (1, 1), taken from a wider buffer: one plane per
    // channel, rows top first
    {
        float rgb[3 * 2 * 2];
        for (int i = 0; i < 12; i++) {
            rgb[i] = static_cast<float>(i);
        }
        CHECK(d.update(1, 1, 2, 2, rgb, 2));
        std::vector<uint8_t> p = read_packet(conn);
        cursor c{ p, 0 };
        CHECK(c.u32() == p.size());
        CHECK(c.u8() == 5);
        CHECK(c.u8() == 0);
        CHECK(c.str() == "img");
        CHECK(c.u32() == 3);
        CHECK(c.str() == "R");
        CHECK(c.str() == "G");
        CHECK(c.str() == "B");
        CHECK(c.u32() == 1);
        CHECK(c.u32() == 1);
        CHECK(c.u32() == 2);
        CHECK(c.u32() == 2);
        const float expect[12] = { 0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11 };
        for (float e : expect) {
            CHECK(c.f32() == e);
        }
        CHECK(c.pos == p.size());
    }

    // the framebuffer's dirty rectangle is what the display thread sends
    {
        framebuffer fb(4, 3);
        std::vector<float> out;
        CHECK(!fb.take_dirty(out).has_value());
        const colorf run[2] = { colorf(1, 2, 3), colorf(4, 5, 6) };
        fb.set_xy_run(1, 2, 2, run);
        fb.set_xy(3, 0, colorf(7, 8, 9));
        std::optional<framebuffer::rect> r = fb.take_dirty(out);
        CHECK(r.has_value());
        if (r) {
            CHECK(r->x == 1 && r->y == 0 && r->w == 3 && r->h == 3);
            CHECK(out.size() == 3 * 3 * 3);
            CHECK(out[2 * 3 * 3 + 0] == 1 && out[2 * 3 * 3 + 5] == 6); // bottom row, px 1 and 2
            CHECK(out[2 * 3] == 7);                                    // top row, px 3
        }
        CHECK(!fb.take_dirty(out).has_value());
        fb.mark_all_dirty();
        r = fb.take_dirty(out);
        CHECK(r && r->w == 4 && r->h == 3);
    }

    // the viewer going away is reported by the next update
    close(conn);
    close(server);
    float px[3] = {};
    bool alive = d.update(0, 0, 1, 1, px, 1);
    if (alive) {
        alive = d.update(0, 0, 1, 1, px, 1); // the first send may land in the socket buffer
    }
    CHECK(!alive);
    CHECK(!d.connected());
    return TEST_RESULT();
}
