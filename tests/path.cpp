#include "path.h"

#include <iomanip>

namespace lhecker_bachelor::path {

bool operator==(const point& lhs, const point& rhs) noexcept {
    constexpr auto kMaxRadiusError = 0.001f;
    return lhs.x == rhs.x && lhs.y == rhs.y && std::abs(lhs.radius - rhs.radius) < kMaxRadiusError;
}

bool operator==(const set& lhs, const set& rhs) noexcept {
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

void operator<<(std::ostream& out, const point& p) {
    out << "point{" << p.x << ", " << p.y << ", " << std::setprecision(8) << p.radius << "}";
}

} // namespace lhecker_bachelor::path
