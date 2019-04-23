#include "_palette.h"

#include <lhecker_bachelor/tracer.h>

void image_bgr2gray(cv::InputArray src, cv::OutputArray dst) {
    tracer trace{"foobar"};

    cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY);

    trace.stop_and_write(dst);
}

// BAD: ~5x slower than blur_box but similar results
void blur_gaussian(cv::InputArray src, cv::OutputArray dst, int kernel_size, double sigma) {
    tracer trace{"foobar"};

    cv::GaussianBlur(src, dst, cv::Size{kernel_size, kernel_size}, sigma, sigma);

    trace.stop_and_write(dst);
}

// BAD: too strong blurring
void blur_pyr_down_up(cv::InputArray src, cv::OutputArray dst) {
    tracer trace{"foobar"};

    cv::Mat tmp;
    cv::pyrDown(src, tmp);

    cv::pyrUp(tmp, dst);

    trace.stop_and_write(dst);
}

// BAD: contrast enhancement is relatively slow and despite that not particular helpful for segmentation
void clahe(cv::InputArray src, cv::OutputArray dst) {
    tracer trace{"foobar"};

    auto clahe = cv::createCLAHE(2.0, cv::Size{8, 8});
    clahe->apply(src, dst);

    trace.stop_and_write(dst);
}

// BAD: Wasn't able to figure dst proper, good parameters which work in most cases.
// BAD: While the colorized version of this algorithm works well, it's GPL which can't be combined with MindObjects.
static void mser_segmentation(cv::InputArray src) {
    tracer trace{"foobar"};

    std::vector<std::vector<cv::Point>> regions;
    std::vector<cv::Rect> mser_bbox;

    auto size = src.size();
    auto ms = cv::MSER::create(20, 100 * 100, (7 * size.width * size.height) / 10, 0.05);
    ms->detectRegions(src, regions, mser_bbox);

    trace.stop();

    if (tracer::has_level(tracer::level::snapshot)) {
        cv::UMat dst;
        cvtColor(src, dst, cv::COLOR_GRAY2BGR);

        for (auto& region : regions) {
            std::vector<cv::Point> hull;
            convexHull(region, hull);
            region = std::move(hull);
        }

        cv::polylines(dst, regions, false, CV_RGB(0, 255, 0));
        trace.stop_and_write(dst);
    }
}

// BAD: _would_ be perfect but is very fragile when there are >2 distinctive luminas ("colors")
void binarize_otsu(cv::InputArray src, cv::OutputArray dst) {
    tracer trace{"foobar"};

    cv::threshold(src, dst, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU); // NOLINT

    trace.stop_and_write(dst);
}

// BAD: significantly worse results than binarize_sauvola
void binarize_niblack(cv::InputArray src, cv::OutputArray dst) {
    tracer trace{"foobar"};

    cv::ximgproc::niBlackThreshold(src, dst, 255, cv::THRESH_BINARY_INV, 19, 0.5, cv::ximgproc::BINARIZATION_NIBLACK);

    trace.stop_and_write(dst);
}

// BAD: has problems with inconsistent dark spots (e.g. when your whiteboard marker is not perfectly covering)
void binarize_adaptive(cv::InputArray src, cv::OutputArray dst, int block_size, int offset) {
    tracer trace{"foobar"};

    cv::adaptiveThreshold(src, dst, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, block_size, offset);

    trace.stop_and_write(dst);
}

// BAD: my own reimplementation is 2x as fast and uses 3x less memory
void binarize_sauvola(cv::InputArray src, cv::OutputArray dst, int kernel_size, double k) {
    tracer trace{"foobar"};

    cv::ximgproc::niBlackThreshold(
        src,
        dst,
        255,
        cv::THRESH_BINARY_INV,
        kernel_size,
        k,
        cv::ximgproc::BINARIZATION_SAUVOLA
    );

    trace.stop_and_write(dst);
}

// BAD: slightly worse results than binarize_sauvola
void binarize_wolf(cv::InputArray src, cv::OutputArray dst) {
    tracer trace{"foobar"};

    cv::ximgproc::niBlackThreshold(src, dst, 255, cv::THRESH_BINARY_INV, 25, 0.3, cv::ximgproc::BINARIZATION_WOLF);

    trace.stop_and_write(dst);
}

// BAD: slightly worse results than binarize_sauvola
void binarize_nick(cv::InputArray src, cv::OutputArray dst) {
    tracer trace{"foobar"};

    cv::ximgproc::niBlackThreshold(src, dst, 255, cv::THRESH_BINARY_INV, 25, -0.2, cv::ximgproc::BINARIZATION_NICK);

    trace.stop_and_write(dst);
}

// BAD: the ximgproc implementation is extremely slow
void binarize_sauvola(cv::InputArray src, cv::OutputArray dst) {
    tracer trace{"foobar"};

    cv::ximgproc::thinning(src, dst, cv::ximgproc::THINNING_SAUVOLA);

    trace.stop_and_write(dst);
}

// BAD: slightly worse results than thinning_zhangsuen
void thinning_guohall(cv::InputArray src, cv::OutputArray dst) {
    tracer trace{"foobar"};

    cv::ximgproc::thinning(src, dst, cv::ximgproc::THINNING_GUOHALL);

    trace.stop_and_write(dst);
}

void connected_components(cv::InputArray src) {
    tracer trace{"foobar"};

    cv::Mat labels;
    cv::Mat stats;
    cv::Mat centroids;
    cv::connectedComponentsWithStats(src, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT);
}

void blur_box(cv::InputArray src, cv::OutputArray dst, int kernel_size) {
    tracer trace{"foobar"};

    cv::blur(src, dst, cv::Size2i{kernel_size, kernel_size});

    trace.stop_and_write(dst);
}

void find_edges(cv::InputArray src, cv::OutputArray dst) {
    tracer trace{"foobar"};

    auto mean = 0.0;
    {
        auto means = cv::mean(src);
        auto mean_count = 0;

        for (auto v : means.val) {
            if (v != 0.0) {
                mean += v;
                ++mean_count;
            }
        }

        mean /= double(mean_count);
    }

    auto upper = std::min(255.0, 1.2 * mean);
    cv::Canny(src, dst, 0, upper);
    cv::dilate(dst, dst, cv::Mat{});

    trace.stop_and_write(dst);
}

// Sort each rectangle's points in the order top-left, top-right, bottom-right, bottom-left.
std::vector<std::vector<cv::Point2i>> find_rectangles(cv::InputArray src) {
    tracer trace{"foobar"};

    std::vector<std::vector<cv::Point2i>> rectangles;

    {
        std::vector<std::vector<cv::Point2i>> contours;
        cv::findContours(src, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        std::vector<cv::Point2i> approx;

        for (auto& contour : contours) {
            approx.clear();

            auto epsilon = 0.02 * cv::arcLength(contour, true);
            cv::approxPolyDP(contour, approx, epsilon, true);

            if (approx.size() != 4 || !cv::isContourConvex(approx) || cv::contourArea(approx) < 100 * 100) {
                continue;
            }

            // Find the maximum angle between joint edges of approx...
            double maxCosine = 0;

            for (int i = 2; i < 5; ++i) {
                auto delta1 = approx[i - 2] - approx[i - 1];
                auto delta2 = approx[i % 4] - approx[i - 1];
                auto cosine = delta1.dot(delta2) / (cv::norm(delta1) * cv::norm(delta2));
                maxCosine = std::max(maxCosine, cosine);
            }

            // ...and filter dst stretched quadrilaterals which don't resemble rectangles anymore.
            if (maxCosine >= 0.3) {
                continue;
            }

            // Fulfill the method's promise of point ordering...

            // First we figure dst the top-left point in approx by finding the one with the smallest
            // manhattan distance to our point of origin (i.e. the top left edge of the image).
            constexpr auto min_distance = std::numeric_limits<int>::max();
            std::size_t top_left_idx = 0;

            for (std::size_t i = 0; i < 4; ++i) {
                auto point = approx[i];
                auto distance = point.x + point.y;
                if (distance < min_distance) {
                    top_left_idx = i;
                }
            }

            auto wrap_mod = [](auto idx, auto max) -> auto {
                return (idx % max + max) % max;
            };

            // Next we'll figure dst whether the list of points in approx is stored clockwise or
            // counterclockwise and thus wether we need to reverse it in the latter case.
            // We'll assume it's correctly clockwise if the element at the position
            // (top_left_idx + 1) is further to the right than (top_left_idx - 1).
            auto next_idx = wrap_mod(top_left_idx + 1, 4);
            auto prev_idx = wrap_mod(top_left_idx - 1, 4);
            auto dir = approx[next_idx].x > approx[prev_idx].x ? 1 : -1;

            std::vector<cv::Point2i>& rectangle = rectangles.emplace_back();
            rectangle.reserve(4);

            // Finally store the points in the correct order
            for (std::size_t i = 0, idx = top_left_idx; i < 4; ++i, idx = wrap_mod(idx + dir, 4)) {
                rectangle.push_back(approx[idx]);
            }
        }
    }

    trace.stop();

    if (tracer::has_level(tracer::level::snapshot)) {
        cv::Mat colorized_rectangles = cv::Mat::zeros(src.size(), CV_8UC3); // NOLINT

        for (int i = 0; i < rectangles.size(); ++i) {
            cv::drawContours(
                colorized_rectangles,
                rectangles,
                i,
                palette_get_mpn65(i),
                2,
                cv::LINE_AA
            );
        }

        trace.stop_and_write(colorized_rectangles);
    }

    return rectangles;
}

void warp_perspective(cv::InputArray src, cv::OutputArray dst, const std::vector<cv::Point2i>& roi_corners) {
    tracer trace{"foobar"};

    cv::Size2i warped_image_size{
        cv::saturate_cast<int>(std::max(
            cv::norm(roi_corners[0] - roi_corners[1]),
            cv::norm(roi_corners[2] - roi_corners[3])
        )),
        cv::saturate_cast<int>(std::max(
            cv::norm(roi_corners[1] - roi_corners[2]),
            cv::norm(roi_corners[3] - roi_corners[0])
        )),
    };

    std::vector<cv::Point2i> dst_corners{4};
    dst_corners[1].x = warped_image_size.width;
    dst_corners[3].x = warped_image_size.width;
    dst_corners[2].y = warped_image_size.height;
    dst_corners[3].y = warped_image_size.height;

    auto M = cv::getPerspectiveTransform(roi_corners, dst_corners);
    cv::warpPerspective(src, dst, M, warped_image_size);

    trace.stop_and_write(dst);
}
