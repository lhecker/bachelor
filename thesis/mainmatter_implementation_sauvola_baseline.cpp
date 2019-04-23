void niBlackThreshold(
    InputArray src, OutputArray dst, double maxValue, int type,
    int blockSize, double k, int binarizationMethod
) {
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
    dst.setTo(maxValue, mask);
}
