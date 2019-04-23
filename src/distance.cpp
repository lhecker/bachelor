#include <lhecker_bachelor/distance.h>

#include <lhecker_bachelor/tracer.h>
#include <opencv2/imgproc.hpp>

namespace lhecker_bachelor::distance {

void transform(cv::InputArray src, cv::OutputArray dst) {
    tracer trace{"lhecker_bachelor::distance::transform"};

    cv::distanceTransform(src, dst, cv::DIST_L2, cv::DIST_MASK_PRECISE, CV_32F);

    trace.stop_and_write(dst);
}

} // namespace lhecker_bachelor::distance
