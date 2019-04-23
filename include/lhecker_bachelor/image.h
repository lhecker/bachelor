#pragma once

#include <string_view>

#include <opencv2/core/mat.hpp>

namespace lhecker_bachelor::image {

void read(const std::string_view& path, cv::OutputArray dst, bool grayscale);
void downscale(cv::InputArray src, cv::OutputArray dst, int preferred_edge_length, int max_edge_length);

} // namespace lhecker_bachelor::image
