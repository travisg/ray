#include <libvec/vector3.h>
#include <sstream>
#include <tests/check.h>

using libvec::vector3d;

static_assert(libvec::dot(vector3d(1, 0, 0), vector3d(1, 0, 0)) == 1.0);

static bool equals(const vector3d &v, double x, double y, double z) {
    return v.x == x && v.y == y && v.z == z;
}

int main() {
    vector3d a(1, 2, 3), b(4, 5, 6);

    CHECK(equals(a + b, 5, 7, 9));
    CHECK(equals(b - a, 3, 3, 3));
    CHECK(equals(-a, -1, -2, -3));
    CHECK(equals(a * b, 4, 10, 18));
    CHECK(equals(a * 2.0, 2, 4, 6));
    CHECK(equals(2.0 * a, 2, 4, 6));
    CHECK(equals(b / 2.0, 2, 2.5, 3));

    vector3d c = a;
    c += b;
    c -= a;
    c *= 2.0;
    c /= 2.0;
    c *= vector3d(1, 1, 1);
    CHECK(equals(c, 4, 5, 6));

    CHECK(dot(a, b) == 32);
    CHECK(a.dot(b) == 32);
    CHECK(equals(cross(vector3d(1, 0, 0), vector3d(0, 1, 0)), 0, 0, 1));

    CHECK(vector3d(3, 4, 0).length_squared() == 25);
    CHECK_NEAR(vector3d(3, 4, 0).length(), 5.0, 1e-12);

    vector3d unit(3, 4, 0);
    unit.normalize();
    CHECK_NEAR(unit.x, 0.6, 1e-12);
    CHECK_NEAR(unit.y, 0.8, 1e-12);

    vector3d ten(3, 4, 0);
    ten.normalize(10);
    CHECK_NEAR(ten.x, 6.0, 1e-12);
    CHECK_NEAR(ten.y, 8.0, 1e-12);

    vector3d zero(0, 0, 0);
    zero.normalize();
    CHECK(equals(zero, 0, 0, 0));

    vector3d splat = 7;
    CHECK(equals(splat, 7, 7, 7));

    std::ostringstream os;
    os << vector3d(1, 2, 3);
    CHECK(os.str() == "[ 1 2 3 ]");

    return TEST_RESULT();
}
