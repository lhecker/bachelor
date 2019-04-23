#pragma once

#include <string_view>
#include <system_error>

namespace lhecker_bachelor::util {

inline void throw_errno() {
    throw std::system_error(errno, std::generic_category());
}

void create_directory(std::string_view path, std::string_view perms_from);

} // namespace lhecker_bachelor::util
