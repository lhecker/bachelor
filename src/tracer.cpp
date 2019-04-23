#include <lhecker_bachelor/tracer.h>

#include <atomic>
#include <iomanip>
#include <iostream>
#include <mutex>

#if __cpp_lib_filesystem
#define FILESYSTEM_ENABLED
#define FILESYSTEM_USE_STD
#include <filesystem>
#elif __has_include(<dirent.h>)
#define FILESYSTEM_ENABLED
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 201800L
#endif
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "defer.h"
#endif

#include <lhecker_bachelor/util.h>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgcodecs.hpp>

#include "string_ext.h"

#ifdef FILESYSTEM_ENABLED
static void replace_non_portable_path_chars(std::string& path) {
    static std::array<uint8_t, 256> valid_path_symbols{{
        /*         0x_0  0x_1  0x_2  0x_3  0x_4  0x_5  0x_6  0x_7  0x_8  0x_9  0x_A  0x_B  0x_C  0x_D  0x_E  0x_F */
        /* 0x0_ */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* 0x1_ */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* 0x2_ */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
        /* 0x3_ */ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* 0x4_ */ 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        /* 0x5_ */ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff,
        /* 0x6_ */ 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        /* 0x7_ */ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* 0x8_ */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* 0x9_ */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* 0xA_ */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* 0xB_ */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* 0xC_ */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* 0xD_ */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* 0xE_ */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* 0xF_ */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    }};

    size_t path_write_idx = 0;
    bool previous_char_was_replaced = false;

    for (auto ch : path) {
        if (valid_path_symbols[ch] == 0) {
            if (previous_char_was_replaced) {
                continue;
            }
            previous_char_was_replaced = true;
            ch = '.';
        } else {
            previous_char_was_replaced = false;
        }

        path[path_write_idx] = ch;
        path_write_idx++;
    }

    path.resize(path_write_idx);
}
#endif // FILESYSTEM_ENABLED

namespace lhecker_bachelor {

tracer::level tracer::m_level = tracer::level::disabled;
std::function<void(const tracer::log_entry&)> tracer::m_log_handler;

void tracer::set_level(tracer::level level) noexcept {
    m_level = level;
}

void tracer::set_log_handler(std::function<void(const tracer::log_entry&)> handler) noexcept {
    m_log_handler = std::move(handler);
}

tracer::tracer(std::string_view title) noexcept : m_title(title) {
    if (has_level(tracer::level::timings)) {
        m_beg = cv::getTickCount();
    }
}

tracer::~tracer() {
    stop();
}

void tracer::stop() noexcept {
    const auto beg = std::exchange(m_beg, 0);
    if (beg == 0) {
        return;
    }

    const auto end = cv::getTickCount();
    const auto delta = 1e3 * double(end - beg) / cv::getTickFrequency();
    const tracer::log_entry entry{m_title, delta};

    if (m_log_handler) {
        m_log_handler(entry);
        return;
    }

    std::cout
        << "TRACE "
        << entry.title
        << ": "
        << std::fixed << entry.millis << "ms\n";
}

void tracer::stop_and_write(cv::InputArray image) {
    stop();

#ifdef FILESYSTEM_ENABLED
    if (!has_level(tracer::level::snapshots)) {
        return;
    }

    static std::once_flag once;
    std::call_once(once, delete_out_files);

    static std::atomic<unsigned int> counter;
    auto id = counter.fetch_add(1);

    std::ostringstream buf;
    buf << "out/trace_"
        << std::setfill('0') << std::setw(4) << id
        << "_"
        << m_title
        << ".tiff";

    auto path = buf.str();
    replace_non_portable_path_chars(path);
    cv::imwrite(path, image);
#else
    (void)(image);
#endif
}

void tracer::delete_out_files() {
#ifdef FILESYSTEM_ENABLED
    auto filename_matches = [](auto name) -> bool {
        return string_starts_with(name, "trace_") && string_ends_with(name, ".tiff");
    };

#ifdef FILESYSTEM_USE_STD
    const std::filesystem::path cwd{"."};
    const std::filesystem::path out{"out"};

    if (std::filesystem::create_directory(out, cwd)) {
        return;
    }

    for (const auto& entry: std::filesystem::directory_iterator(out)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto filename = entry.path().filename();
        const auto filename_str = filename.string();

        if (!filename_matches(filename_str)) {
            continue;
        }

        std::filesystem::remove(entry);
    }
#else // FILESYSTEM_USE_STD
    // TODO(lhecker): The following code may be removed as soon as compilers have fully catched up to C++17/20.
    const std::string_view out{"out"};
    auto dir = opendir(out.data());
    if (!dir && errno == ENOENT) {
        util::create_directory(out, ".");
        return;
    }
    if (!dir) {
        util::throw_errno();
    }
    defer {
        closedir(dir);
    };

    while (true) {
        auto dp = readdir(dir);
        if (!dp) {
            break;
        }

        std::string_view name(dp->d_name);
        if (name.empty() || name[0] == '.') {
            continue;
        }

        if (!filename_matches(name)) {
            continue;
        }

        std::string path;
        path.reserve(out.size() + 1 + name.size());
        path += out;
        path += "/";
        path += name;

        struct stat statbuf{};
        if (stat(path.data(), &statbuf)) {
            continue;
        }
        if (!S_ISREG(statbuf.st_mode)) {
            continue;
        }

        unlink(path.data());
    }
#endif // FILESYSTEM_USE_STD
#endif // FILESYSTEM_ENABLED
}

} // namespace lhecker_bachelor
