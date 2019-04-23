void distance_transform_brute_force(const cv::Mat& src, cv::Mat& dst) {
    dst.create(src.size(), CV_32FC1);
    dst.setTo(cv::Scalar(0));

    src.forEach<uint8_t>([&](uint8_t& src_px, const int src_pos[]) {
        if (!src_px) {
            return;
        }

        constexpr auto max = std::numeric_limits<int>::max();
        int min_dist = max;

        for (int y = 0; y < src.rows; ++y) {
            const auto row = src.ptr<uint8_t>(y, 0);

            for (int x = 0; x < src.cols; ++x) {
                if (row[x]) {
                    continue;
                }

                const auto dy = y - src_pos[0];
                const auto dx = x - src_pos[1];
                const auto dist = dx * dx + dy * dy;

                if (dist < min_dist && dist != 0) {
                    min_dist = dist;
                }
            }
        }

        if (min_dist != max) {
            dst.at<float>(src_pos) = std::sqrt(float(min_dist));
        }
    });
}
