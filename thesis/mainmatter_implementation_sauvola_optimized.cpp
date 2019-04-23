void niBlackThreshold(
    InputArray src, OutputArray dst, double maxValue, int type,
    int blockSize, double k, int binarizationMethod
) {
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
