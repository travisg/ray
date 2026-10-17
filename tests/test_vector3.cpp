#include "check.h"
#include <libvec/vector3.h>
#include <sstream>

using Libvec::Vector3d;

static_assert(Libvec::Dot(Vector3d(1, 0, 0), Vector3d(1, 0, 0)) == 1.0);

static bool Equals(const Vector3d &v, double x, double y, double z) {
    return v.getx() == x && v.gety() == y && v.getz() == z;
}

int main() {
    Vector3d a(1, 2, 3), b(4, 5, 6);

    CHECK(Equals(a + b, 5, 7, 9));
    CHECK(Equals(b - a, 3, 3, 3));
    CHECK(Equals(-a, -1, -2, -3));
    CHECK(Equals(a * b, 4, 10, 18));
    CHECK(Equals(a * 2.0, 2, 4, 6));
    CHECK(Equals(2.0 * a, 2, 4, 6));
    CHECK(Equals(b / 2.0, 2, 2.5, 3));

    Vector3d c = a;
    c += b;
    c -= a;
    c *= 2.0;
    c /= 2.0;
    c *= Vector3d(1, 1, 1);
    CHECK(Equals(c, 4, 5, 6));

    CHECK(Dot(a, b) == 32);
    CHECK(a.Dot(b) == 32);
    CHECK(Equals(Cross(Vector3d(1, 0, 0), Vector3d(0, 1, 0)), 0, 0, 1));

    CHECK(Vector3d(3, 4, 0).LengthSquared() == 25);
    CHECK_NEAR(Vector3d(3, 4, 0).Length(), 5.0, 1e-12);

    Vector3d unit(3, 4, 0);
    unit.Normalize();
    CHECK_NEAR(unit.getx(), 0.6, 1e-12);
    CHECK_NEAR(unit.gety(), 0.8, 1e-12);

    Vector3d ten(3, 4, 0);
    ten.Normalize(10);
    CHECK_NEAR(ten.getx(), 6.0, 1e-12);
    CHECK_NEAR(ten.gety(), 8.0, 1e-12);

    Vector3d zero(0, 0, 0);
    zero.Normalize();
    CHECK(Equals(zero, 0, 0, 0));

    Vector3d splat = 7;
    CHECK(Equals(splat, 7, 7, 7));

    std::ostringstream os;
    os << Vector3d(1, 2, 3);
    CHECK(os.str() == "[ 1 2 3 ]");

    return TEST_RESULT();
}
