#pragma once

#include <lhecker_bachelor/path.h>

namespace lhecker_bachelor::path {

bool operator==(const point& lhs, const point& rhs) noexcept;
bool operator==(const set& lhs, const set& rhs) noexcept;
void operator<<(std::ostream& out, const point& p);

} // namespace lhecker_bachelor::path
