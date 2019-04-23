#include "path.h"

#include <gtest/gtest.h>
#include <opencv2/core/mat.hpp>

constexpr uint8_t _ = 0x00;
constexpr uint8_t W = 0xff;

static lhecker_bachelor::path::set search(const cv::Mat& skeleton) {
    const cv::Mat distance(skeleton.size(), CV_32FC1, cv::Scalar(0)); // NOLINT
    return lhecker_bachelor::path::set::search(skeleton, distance);
}

TEST(path_set_search, empty) {
    const lhecker_bachelor::path::set expected{
    };
    const cv::Mat skeleton({3, 3}, {
        _, _, _,
        _, _, _,
        _, _, _,
    });
    const auto actual = search(skeleton);
    EXPECT_EQ(actual, expected);
}

TEST(path_set_search, dot) {
    const lhecker_bachelor::path::set expected{
        {
            {1, 1, 0},
        },
    };
    const cv::Mat skeleton({3, 3}, {
        _, _, _,
        _, W, _,
        _, _, _,
    });
    const auto actual = search(skeleton);
    EXPECT_EQ(actual, expected);
}

TEST(path_set_search, diagonal) {
    const lhecker_bachelor::path::set expected{
        {
            {1, 1, 0},
            {2, 2, 0},
        },
    };
    const cv::Mat skeleton({3, 3}, {
        _, _, _,
        _, W, _,
        _, _, W,
    });
    const auto actual = search(skeleton);
    EXPECT_EQ(actual, expected);
}

TEST(path_set_search, two) {
    const lhecker_bachelor::path::set expected{
        {
            {1, 0, 0},
            {2, 0, 0},
            {3, 0, 0},
            {3, 1, 0},
            {3, 2, 0},
            {2, 2, 0},
            {1, 2, 0},
            {1, 3, 0},
            {1, 4, 0},
            {2, 4, 0},
            {3, 4, 0},
        },
    };
    const cv::Mat skeleton({5, 5}, {
        _, W, W, W, _,
        _, _, _, W, _,
        _, W, W, W, _,
        _, W, _, _, _,
        _, W, W, W, _,
    });
    const auto actual = search(skeleton);
    EXPECT_EQ(actual, expected);
}

TEST(path_set_search, three) {
    const lhecker_bachelor::path::set expected{
        {
            {1, 0, 0},
            {2, 0, 0},
            {3, 0, 0},
            {3, 1, 0},
            {3, 2, 0},
        },
        {
            {1, 2, 0},
            {2, 2, 0},
            {3, 2, 0},
        },
        {
            {3, 2, 0},
            {3, 3, 0},
            {3, 4, 0},
            {2, 4, 0},
            {1, 4, 0},
        },
    };
    const cv::Mat skeleton({5, 5}, {
        _, W, W, W, _,
        _, _, _, W, _,
        _, W, W, W, _,
        _, _, _, W, _,
        _, W, W, W, _,
    });
    const auto actual = search(skeleton);
    EXPECT_EQ(actual, expected);
}

TEST(path_set_search, square) {
    const lhecker_bachelor::path::set expected{
        {
            {1, 1, 0},
            {2, 1, 0},
            {3, 1, 0},
            {3, 2, 0},
            {3, 3, 0},
            {2, 3, 0},
            {1, 3, 0},
            {1, 2, 0},
            {1, 1, 0},
        }
    };
    const cv::Mat skeleton({5, 5}, {
        _, _, _, _, _,
        _, W, W, W, _,
        _, W, _, W, _,
        _, W, W, W, _,
        _, _, _, _, _,
    });
    const auto actual = search(skeleton);
    EXPECT_EQ(actual, expected);
}
