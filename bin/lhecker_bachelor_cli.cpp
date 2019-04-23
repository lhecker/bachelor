#include "lhecker_bachelor_cli.h"

#include <fstream>
#include <opencv2/opencv.hpp>

#include <lhecker_bachelor/binarize.h>
#include <lhecker_bachelor/distance.h>
#include <lhecker_bachelor/image.h>
#include <lhecker_bachelor/path.h>
#include <lhecker_bachelor/skeleton.h>
#include <lhecker_bachelor/tracer.h>

int main(int argc, char* argv[]) {
    auto keys =
        "{help h||Print this message}"
        "{dir||Output directory}"
        "{max-resolution|1920|Images are downscaled to the given edge length}"
        "{sauvola-size|41|Windows size of Sauvola's Binarization Method}"
        "{sauvola-factor|0.3|Factor k in Sauvola's Binarization Method}"
        "{max-simplification-error|2.0|Maximum pixel error to which lines are simplified}"
        "{debug|false|Enable writing of debug files}"
        "{@input|<none>|Input image}";
    cv::CommandLineParser app(argc, argv, keys);

    if (app.has("help")) {
        app.printMessage();
        return 0;
    }

    const auto input_path = app.get<std::string>("@input");
    const auto output_dir = app.get<std::string>("dir");
    const auto output_path = make_output_path(input_path, output_dir);

    const auto max_resolution = app.get<int>("max-resolution");
    const auto sauvola_size = app.get<int>("sauvola-size");
    const auto sauvola_factor = app.get<double>("sauvola-factor");
    const auto max_simplification_error = app.get<double>("max-simplification-error");
    const auto debug = app.get<bool>("debug");

    lhecker_bachelor::tracer::set_level(
        debug
            ? lhecker_bachelor::tracer::level::timings | lhecker_bachelor::tracer::level::snapshots
            : lhecker_bachelor::tracer::level::timings
    );
    lhecker_bachelor::tracer trace{"main"};

    try {
        cv::UMat input;
        cv::Mat distance;
        cv::Mat skeleton;

        lhecker_bachelor::image::read(input_path, input, true);

        if (max_resolution > 0) {
            lhecker_bachelor::image::downscale(input, input, max_resolution, max_resolution);
        }

        lhecker_bachelor::binarize::sauvola(input, input, sauvola_size, sauvola_factor, cv::THRESH_BINARY_INV);
        lhecker_bachelor::distance::transform(input, distance);
        lhecker_bachelor::skeleton::zhangsuen(input, skeleton);

        auto paths = lhecker_bachelor::path::set::search(skeleton, distance);
        paths.apply_approximation(max_simplification_error);

        auto svg = paths.generate_svg(skeleton.size());

        std::ofstream out(output_path, std::ios::binary);
        out.write(svg.data(), svg.size());
    } catch (const std::exception& e) {
        std::cerr << "exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "unknown exception" << std::endl;
    }

    return 0;
}
