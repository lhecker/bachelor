#include <iostream>

#include <opencv2/core/ocl.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/ximgproc.hpp>

#include <lhecker_bachelor/binarize.h>
#include <lhecker_bachelor/image.h>
#include <lhecker_bachelor/skeleton.h>
#include <lhecker_bachelor/tracer.h>

inline bool zhangsuen_condition(const cv::Mat& src, int i, int j, int iter) {
    uchar p2 = src.at<uchar>(i - 1, j);
    uchar p3 = src.at<uchar>(i - 1, j + 1);
    uchar p4 = src.at<uchar>(i, j + 1);
    uchar p5 = src.at<uchar>(i + 1, j + 1);
    uchar p6 = src.at<uchar>(i + 1, j);
    uchar p7 = src.at<uchar>(i + 1, j - 1);
    uchar p8 = src.at<uchar>(i, j - 1);
    uchar p9 = src.at<uchar>(i - 1, j - 1);

    int A = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
        (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
        (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
        (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
    int B = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
    int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
    int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

    return A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0;
}

void thinning_opencv(cv::InputArray input, cv::OutputArray output) {
    lhecker_bachelor::tracer trace{__FUNCTION__};

    static auto iteration = [](cv::Mat& src, int iter) {
        cv::Mat marker = cv::Mat::zeros(src.size(), CV_8UC1);

        for (int i = 1; i < src.rows - 1; i++) {
            for (int j = 1; j < src.cols - 1; j++) {
                if (zhangsuen_condition(src, i, j, iter)) {
                    marker.at<uchar>(i, j) = 1;
                }
            }
        }

        src &= ~marker;
    };

    cv::Mat dst = input.getMat().clone();
    dst /= 255;

    cv::Mat prev = cv::Mat::zeros(dst.size(), CV_8UC1);
    cv::Mat diff;

    do {
        iteration(dst, 0);
        iteration(dst, 1);
        cv::absdiff(dst, prev, diff);
        dst.copyTo(prev);
    } while (cv::countNonZero(diff) > 0);

    dst *= 255;
    output.assign(dst);
}

void thinning_without_diff(cv::InputArray input, cv::OutputArray output) {
    lhecker_bachelor::tracer trace{__FUNCTION__};

    static auto iteration = [](cv::Mat& src, int iter) -> bool {
        cv::Mat marker = cv::Mat::zeros(src.size(), CV_8UC1);
        bool has_changes = false;

        for (int i = 1; i < src.rows - 1; i++) {
            for (int j = 1; j < src.cols - 1; j++) {
                if (zhangsuen_condition(src, i, j, iter)) {
                    marker.at<uchar>(i, j) = 1;

                    if (src.at<uchar>(i, j) != 0) {
                        has_changes = true;
                    }
                }
            }
        }

        src &= ~marker;
        return has_changes;
    };

    cv::Mat dst = input.getMat().clone();
    dst /= 255;

    cv::Mat prev = cv::Mat::zeros(dst.size(), CV_8UC1);
    cv::Mat diff;
    bool has_changes;

    while (true) {
        has_changes = false;
        has_changes |= iteration(dst, 0);
        has_changes |= iteration(dst, 1);

        if (!has_changes) {
            break;
        }

        dst.copyTo(prev);
    }

    dst *= 255;
    output.assign(dst);
}

void thinning_without_markers(cv::InputArray input, cv::OutputArray output) {
    lhecker_bachelor::tracer trace{__FUNCTION__};

    static auto iteration = [](const cv::Mat& src, cv::Mat& dst, int iter) -> bool {
        bool has_changes = false;

        for (int i = 1; i < src.rows - 1; i++) {
            for (int j = 1; j < src.cols - 1; j++) {
                if (zhangsuen_condition(src, i, j, iter) && src.at<uchar>(i, j) != 0) {
                    dst.at<uchar>(i, j) = 0;
                    has_changes = true;
                }
            }
        }

        return has_changes;
    };

    cv::Mat dst = input.getMat().clone();
    dst /= 255;

    cv::Mat prev(dst.size(), CV_8UC1);
    bool has_changes;

    do {
        has_changes = false;
        dst.copyTo(prev);
        has_changes |= iteration(prev, dst, 0);
        dst.copyTo(prev);
        has_changes |= iteration(prev, dst, 1);
    } while (has_changes);

    dst *= 255;
    output.assign(dst);
}

void thinning_do_less(cv::InputArray input, cv::OutputArray output) {
    lhecker_bachelor::tracer trace{__FUNCTION__};

    static auto iteration = [](const cv::Mat& src, cv::Mat& dst, int iter) -> bool {
        bool has_changes = false;

        for (int i = 1; i < src.rows - 1; i++) {
            for (int j = 1; j < src.cols - 1; j++) {
                if (src.at<uchar>(i, j) != 0 && zhangsuen_condition(src, i, j, iter)) {
                    dst.at<uchar>(i, j) = 0;
                    has_changes = true;
                }
            }
        }

        return has_changes;
    };

    cv::Mat dst = input.getMat().clone();
    dst /= 255;

    cv::Mat prev(dst.size(), CV_8UC1);
    bool has_changes;

    do {
        has_changes = false;
        dst.copyTo(prev);
        has_changes |= iteration(prev, dst, 0);
        dst.copyTo(prev);
        has_changes |= iteration(prev, dst, 1);
    } while (has_changes);

    dst *= 255;
    output.assign(dst);
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
    cv::Mat output;

    try {
        lhecker_bachelor::image::read(input_path, input, true);
    } catch (...) {
        std::cerr << "Cannot read image file at " << input_path << std::endl;
        return 2;
    }

    lhecker_bachelor::binarize::sauvola(input, input, 41, 0.3, cv::THRESH_BINARY_INV);

    lhecker_bachelor::tracer::set_level(lhecker_bachelor::tracer::level::timings);
    thinning_opencv(input, output);
    thinning_without_diff(input, output);
    thinning_without_markers(input, output);
    thinning_do_less(input, output);
    lhecker_bachelor::skeleton::zhangsuen(input, output);

    return 0;
}
