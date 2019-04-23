#include <lhecker_bachelor/image.h>

#include <lhecker_bachelor/tracer.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace lhecker_bachelor::image {

void read(const std::string_view& path, cv::OutputArray dst, bool grayscale) {
    tracer trace{"lhecker_bachelor::image::read"};

    auto img = cv::imread({path.data(), path.size()}, grayscale ? cv::IMREAD_GRAYSCALE : cv::IMREAD_COLOR);
    if (img.empty()) {
        std::string what{"cannot read image file at "};
        what += path;
        throw std::invalid_argument(what);
    }

    dst.assign(img);
}

void downscale(cv::InputArray src, cv::OutputArray dst, int preferred_edge_length, int max_edge_length) {
    tracer trace{"lhecker_bachelor::image::downscale"};

    auto size = src.size();

    if (size.width <= max_edge_length && size.height <= max_edge_length) {
        switch (src.kind()) {
            case cv::_InputArray::MAT:
                dst.assign(src.getMat());
                return;
            case cv::_InputArray::UMAT:
                dst.assign(src.getUMat());
                return;
            default:
                src.copyTo(dst);
                return;
        }
    }

    if (size.width > size.height) {
        size.height = (preferred_edge_length * size.height) / size.width;
        size.width = preferred_edge_length;
    } else {
        size.width = (preferred_edge_length * size.width) / size.height;
        size.height = preferred_edge_length;
    }

    cv::resize(src, dst, size, 0, 0, cv::INTER_AREA);
    trace.stop_and_write(dst);
}

} // namespace lhecker_bachelor::image
