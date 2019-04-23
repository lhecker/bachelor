#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

namespace lhecker_bachelor::binarize {

void sauvola(cv::InputArray src, cv::OutputArray dst, int kernel_size, double k, cv::ThresholdTypes threshold_type);

} // namespace lhecker_bachelor::binarize
