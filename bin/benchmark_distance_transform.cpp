#include <iostream>

#include <lhecker_bachelor/distance.h>
#include <lhecker_bachelor/image.h>
#include <lhecker_bachelor/tracer.h>
#include <opencv2/core/utility.hpp>

void distance_transform_brute_force(const cv::Mat& src, cv::Mat& dst) {
    CV_Assert(src.type() == CV_8UC1);

    lhecker_bachelor::tracer trace{"brute_force_distance_transform"};

    dst.create(src.size(), CV_32FC1);
    dst.setTo(cv::Scalar(0));

    src.forEach<uint8_t>([&](uint8_t& src_px, const int src_pos[]) {
        if (!src_px) {
            return;
        }

        constexpr auto max = std::numeric_limits<int>::max();
        int min_square_distance = max;

        for (int y = 0; y < src.rows; ++y) {
            const auto row = src.ptr<uint8_t>(y, 0);

            for (int x = 0; x < src.cols; ++x) {
                if (row[x]) {
                    continue;
                }

                const auto dy = y - src_pos[0];
                const auto dx = x - src_pos[1];
                const auto square_distance = dx * dx + dy * dy;

                if (square_distance < min_square_distance && square_distance != 0) {
                    min_square_distance = square_distance;
                }
            }
        }

        if (min_square_distance != max) {
            dst.at<float>(src_pos) = std::sqrt(float(min_square_distance));
        }
    });

    trace.stop_and_write(dst);
}

int main(int argc, char* argv[]) {
    auto keys =
        "{help h||print this message}"
        "{@input|<none>|input image}";
    cv::CommandLineParser app(argc, argv, keys);

    if (app.has("help")) {
        app.printMessage();
        return 0;
    }

    const auto input_path = app.get<std::string>("@input");
    cv::Mat input;

    try {
        lhecker_bachelor::image::read(input_path, input, true);
    } catch (...) {
        std::cerr << "Cannot read image file at " << input_path << std::endl;
        return 2;
    }

    lhecker_bachelor::tracer::set_level(
        lhecker_bachelor::tracer::level::timings | lhecker_bachelor::tracer::level::snapshots
    );

    cv::Mat output;
    distance_transform_brute_force(input, output);
    lhecker_bachelor::distance::transform(input, output);
}
