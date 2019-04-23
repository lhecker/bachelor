#include "lhecker_bachelor_cli.h"

#if !has_cpp_lib_filesystem

#include <string>

#if defined _WIN32 || defined __CYGWIN__
#define preferred_separator "\\"
#else
#define preferred_separator "/"
#endif

std::string_view basename(std::string_view path) {
    auto idx = path.find_last_of(preferred_separator);
    return idx != std::string_view::npos ? path.substr(idx + 1) : path;
}

std::string_view strip_extension(std::string_view path) {
    auto idx = path.find_last_of(preferred_separator ".");
    return idx != std::string_view::npos && path[idx] == '.' ? path.substr(0, idx) : path;
}

std::string make_output_path(std::string_view input_path, std::string_view output_dir) {
    const auto without_ext = strip_extension(input_path);
    std::string output_path;

    if (output_dir.empty()) {
        output_path.append(without_ext);
    } else {
        const auto basename_without_ext = basename(without_ext);
        output_path.append(output_dir);
        output_path.append(preferred_separator);
        output_path.append(basename_without_ext);
    }

    output_path.append(".svg");
    return output_path;
}

#endif // !has_cpp_lib_filesystem
