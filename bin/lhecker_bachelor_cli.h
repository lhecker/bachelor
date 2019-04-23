#pragma once

#include <string_view>

#if __has_include(<filesystem>) && __cpp_lib_filesystem >= 201703
#define has_cpp_lib_filesystem 1
#endif

std::string make_output_path(std::string_view input_path, std::string_view output_dir);
