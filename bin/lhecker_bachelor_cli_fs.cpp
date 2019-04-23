#include "lhecker_bachelor_cli.h"

#if has_cpp_lib_filesystem

#include <filesystem>

std::string make_output_path(std::string_view input_path, std::string_view output_dir) {
    std::filesystem::path output_path{input_path};
    output_path.replace_extension(".svg");

    if (!output_dir.empty()) {
        output_path = output_dir / output_path.filename();
    }

    return output_path.string();
}

#endif // has_cpp_lib_filesystem
