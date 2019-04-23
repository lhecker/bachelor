#include <lhecker_bachelor/skeleton.h>

#include <numeric>

#include <lhecker_bachelor/tracer.h>
#include <opencv2/core.hpp>

namespace lhecker_bachelor::skeleton {

static bool zhangsuen_iteration(const cv::Mat& src, cv::Mat& dst, bool second_pass) {
    const auto dstdata = dst.data;
    bool has_changes = false;

    for (auto p = src.datastart; p != src.dataend; ++p) {
        const auto remaining = src.dataend - p;

        // Searching for the next white pixel is the perfect task for memchr().
        // That function is usually highly optimized in any modern compiler/stdlib
        // and will be faster than anything I could come up with.
        p = static_cast<const uint8_t*>(memchr(p, 255, remaining));
        if (!p) {
            break;
        }

        const auto p0 = p - 1 - src.cols;
        const auto p1 = p - 1;
        const auto p2 = p - 1 + src.cols;

        // Other Zhang-Suen implementations use <2 and >6 here.
        // We add +1 because our `data` array also contains the white pixel in the center at `p`.
        const auto white_pixels = 0
            + p0[0] + p0[1] + p0[2]
            + p1[0] + p1[1] + p1[2]
            + p2[0] + p2[1] + p2[2];
        if (white_pixels < 3 * 255 || white_pixels > 7 * 255) {
            continue;
        }

        // If we don't just search for black -> white transitions like the paper suggests,
        // but also search for white -> black transitions we can leverage the faster XOR instruction.
        const auto transitions_black_white = 0
            + int(p0[0] ^ p0[1])
            + int(p0[1] ^ p0[2])
            + int(p0[2] ^ p1[2])
            + int(p1[2] ^ p2[2])
            + int(p2[2] ^ p2[1])
            + int(p2[1] ^ p2[0])
            + int(p2[0] ^ p1[0])
            + int(p1[0] ^ p0[0]);

        // But due to that we need to check whether 2 transitions occurred and not just one.
        if (transitions_black_white != 2 * 255) {
            continue;
        }

        if (second_pass) {
            if ((p0[1] + p1[0] + p1[2]) == 3 * 255 ||
                (p0[1] + p1[0] + p2[1]) == 3 * 255) {
                continue;
            }
        } else {
            if ((p0[1] + p1[2] + p2[1]) == 3 * 255 ||
                (p1[0] + p1[2] + p2[1]) == 3 * 255) {
                continue;
            }
        }

        const auto srcoffset = p - src.datastart;
        dstdata[srcoffset] = 0;
        has_changes = true;
    }

    return has_changes;
}

void zhangsuen(cv::InputArray src, cv::OutputArray dst) {
    CV_Assert(src.type() == CV_8UC1); // NOLINT
    CV_Assert(dst.kind() == cv::_InputArray::MAT);

    tracer trace{"lhecker_bachelor::skeleton::zhangsuen"};

    cv::Mat mat;
    cv::Mat ref;
    bool has_changes;

    cv::copyMakeBorder(src, mat, 1, 1, 1, 1, cv::BORDER_CONSTANT | cv::BORDER_ISOLATED, cv::Scalar{});
    CV_Assert(mat.isContinuous());

    ref.create(mat.size(), CV_8UC1);
    CV_Assert(ref.isContinuous());

    do {
        has_changes = false;
        mat.copyTo(ref);
        has_changes |= zhangsuen_iteration(ref, mat, false);
        mat.copyTo(ref);
        has_changes |= zhangsuen_iteration(ref, mat, true);
    } while (has_changes);

    dst.assign(mat.adjustROI(-1, -1, -1, -1));
    trace.stop_and_write(dst);
}

static bool zhangwang_iteration(const cv::Mat& src, cv::Mat& dst) {
    const auto dstdata = dst.data;
    bool has_changes = false;

    for (auto p = src.datastart; p != src.dataend; ++p) {
        const auto remaining = src.dataend - p;

        // Searching for the next white pixel is the perfect task for memchr().
        // That function is usually highly optimized in any modern compiler/stdlib
        // and will be faster than anything I could come up with.
        p = static_cast<const uint8_t*>(memchr(p, 255, remaining));
        if (!p) {
            break;
        }

        const auto p0 = p - 1 - src.cols;
        const auto p1 = p - 1;
        const auto p2 = p - 1 + src.cols;

        // Other Zhang-Wang implementations use <2 and >6 here.
        // We add +1 because our `data` array also contains the white pixel in the center at `p`.
        const auto white_pixels = 0
            + p0[0] + p0[1] + p0[2]
            + p1[0] + p1[1] + p1[2]
            + p2[0] + p2[1] + p2[2];
        if (white_pixels < 3 * 255 || white_pixels > 7 * 255) {
            continue;
        }

        // If we don't just search for black -> white transitions like the paper suggests,
        // but also search for white -> black transitions we can leverage the faster XOR instruction.
        const auto transitions_black_white = 0
            + int(p0[0] ^ p0[1])
            + int(p0[1] ^ p0[2])
            + int(p0[2] ^ p1[2])
            + int(p1[2] ^ p2[2])
            + int(p2[2] ^ p2[1])
            + int(p2[1] ^ p2[0])
            + int(p2[0] ^ p1[0])
            + int(p1[0] ^ p0[0]);

        // But due to that we need to check whether 2 transitions occurred and not just one.
        if (transitions_black_white != 2 * 255) {
            continue;
        }

        if (((p0[1] + p1[0] + p1[2]) == 3 * 255 && *(p - 2 * src.cols) == 0) ||
            ((p0[1] + p1[2] + p2[1]) == 3 * 255 && *(p + 3) == 0)) {
            continue;
        }

        const auto srcoffset = p - src.datastart;
        dstdata[srcoffset] = 0;
        has_changes = true;
    }

    return has_changes;
}

void zhangwang(cv::InputArray src, cv::OutputArray dst) {
    CV_Assert(src.type() == CV_8UC1); // NOLINT
    CV_Assert(dst.kind() == cv::_InputArray::MAT);

    tracer trace{"lhecker_bachelor::skeleton::zhangwang"};

    cv::Mat mat;
    cv::Mat ref;
    bool has_changes;

    cv::copyMakeBorder(src, mat, 2, 1, 1, 2, cv::BORDER_CONSTANT | cv::BORDER_ISOLATED, cv::Scalar{});
    CV_Assert(mat.isContinuous());

    ref.create(mat.size(), CV_8UC1);
    CV_Assert(ref.isContinuous());

    do {
        mat.copyTo(ref);
        has_changes = zhangwang_iteration(ref, mat);
    } while (has_changes);

    dst.assign(mat.adjustROI(-2, -1, -1, -2));
    trace.stop_and_write(dst);
}

} // namespace lhecker_bachelor::skeleton
