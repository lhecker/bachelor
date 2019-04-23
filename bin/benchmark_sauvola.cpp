#include <iostream>

#include <opencv2/core/ocl.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/ximgproc.hpp>

#include <lhecker_bachelor/binarize.h>
#include <lhecker_bachelor/image.h>
#include <lhecker_bachelor/tracer.h>

void sauvola_opencv(cv::InputArray src, cv::OutputArray dst, int blockSize, double k, cv::ThresholdTypes type) {
    lhecker_bachelor::tracer trace{__FUNCTION__};

    cv::Mat mean, sqmean, stddev, variance, thresh, mask;

    cv::boxFilter(
        src, mean, CV_32F, cv::Size(blockSize, blockSize),
        cv::Point(-1, -1), true, cv::BORDER_REPLICATE
    );
    cv::sqrBoxFilter(
        src, sqmean, CV_32F, cv::Size(blockSize, blockSize),
        cv::Point(-1, -1), true, cv::BORDER_REPLICATE
    );

    variance = sqmean - mean.mul(mean);
    cv::sqrt(variance, stddev);

    thresh = mean.mul(1. + k * (stddev / 128.0 - 1.));
    thresh.convertTo(thresh, src.depth());

    cv::compare(
        src, thresh, mask,
        type == cv::THRESH_BINARY ? cv::CMP_GT : cv::CMP_LE
    );

    dst.create(src.size(), src.type());
    dst.setTo(0);
    dst.setTo(255.0, mask);
}

void sauvola_less_cv_mats(cv::InputArray src, cv::OutputArray dst, int blockSize, double k, cv::ThresholdTypes type) {
    lhecker_bachelor::tracer trace{__FUNCTION__};

    cv::Mat mean, sqmean, thresh;
    cv::boxFilter(
        src, mean, CV_32F, cv::Size(blockSize, blockSize),
        cv::Point(-1, -1), true, cv::BORDER_REPLICATE
    );
    cv::sqrBoxFilter(
        src, sqmean, CV_32F, cv::Size(blockSize, blockSize),
        cv::Point(-1, -1), true, cv::BORDER_REPLICATE
    );

    thresh = sqmean - mean.mul(mean);
    cv::sqrt(thresh, thresh);

    thresh = mean.mul(1. + k * (thresh / 128.0 - 1.));
    thresh.convertTo(thresh, src.depth());

    cv::compare(
        src, thresh, dst,
        type == cv::THRESH_BINARY ? cv::CMP_GT : cv::CMP_LE
    );
}

void sauvola_umat(cv::InputArray src, cv::OutputArray dst, int blockSize, double k, cv::ThresholdTypes type) {
    lhecker_bachelor::tracer trace{__FUNCTION__};

    cv::Mat mean, sqmean, thresh;

    cv::boxFilter(
        src, mean, CV_32F, cv::Size(blockSize, blockSize),
        cv::Point(-1, -1), true, cv::BORDER_REPLICATE
    );
    cv::sqrBoxFilter(
        src, sqmean, CV_32F, cv::Size(blockSize, blockSize),
        cv::Point(-1, -1), true, cv::BORDER_REPLICATE
    );

    cv::multiply(mean, mean, thresh);
    cv::subtract(sqmean, thresh, thresh);
    cv::sqrt(thresh, thresh);

    thresh.convertTo(thresh, -1, k / 128.0, 1.0 - k);
    cv::multiply(mean, thresh, thresh);
    thresh.convertTo(thresh, src.depth());

    cv::compare(
        src, thresh, dst,
        type == cv::THRESH_BINARY ? cv::CMP_GT : cv::CMP_LE
    );
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
    cv::UMat input;
    cv::UMat output;

    try {
        lhecker_bachelor::image::read(input_path, input, true);
    } catch (...) {
        std::cerr << "Cannot read image file at " << input_path << std::endl;
        return 2;
    }

    for (int i = 0; i < 2; ++i) {
        if (i == 1) {
            lhecker_bachelor::tracer::set_level(lhecker_bachelor::tracer::level::timings);
        }

        sauvola_opencv(input, output, 41, 0.3, cv::THRESH_BINARY_INV);
        sauvola_less_cv_mats(input, output, 41, 0.3, cv::THRESH_BINARY_INV);
        sauvola_umat(input, output, 41, 0.3, cv::THRESH_BINARY_INV);
        lhecker_bachelor::binarize::sauvola(input, output, 41, 0.3, cv::THRESH_BINARY_INV);
    }

    return 0;
}
