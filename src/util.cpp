#include <lhecker_bachelor/util.h>

#if __cpp_lib_filesystem
#define FILESYSTEM_ENABLED
#define FILESYSTEM_USE_STD
#include <filesystem>
#elif __has_include(<dirent.h>)
#define FILESYSTEM_ENABLED
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 201800L
#endif
#include <sys/stat.h>
#endif

namespace lhecker_bachelor::util {

void create_directory(std::string_view path, std::string_view perms_from) {
#ifdef FILESYSTEM_ENABLED
#ifdef FILESYSTEM_USE_STD
    std::filesystem::create_directory(path, perms_from);
#else
    struct stat statbuf{};
    if (stat(perms_from.data(), &statbuf)) {
        throw_errno();
    }

    auto ret = mkdir(path.data(), mode_t(statbuf.st_mode & 0777));
    if (ret && errno != EEXIST) {
        throw_errno();
    }
#endif
#endif
}

} // namespace lhecker_bachelor::util
