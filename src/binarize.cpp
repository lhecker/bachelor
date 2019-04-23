#include <lhecker_bachelor/binarize.h>

#include <lhecker_bachelor/tracer.h>

namespace lhecker_bachelor::binarize {

void sauvola(cv::InputArray src, cv::OutputArray dst, int kernel_size, double k, cv::ThresholdTypes threshold_type) {
    CV_Assert(src.type() == CV_8UC1); // NOLINT
    CV_Assert(kernel_size % 2 == 1 && kernel_size > 1);
    CV_Assert(threshold_type == cv::THRESH_BINARY || threshold_type == cv::THRESH_BINARY_INV);

    tracer trace{"lhecker_bachelor::binarize::sauvola"};

    // The common formula you can find for Sauvola's method is:
    //   T = m(x,y) * (1 + k * ((s(x,y) / R) - 1))
    // where:
    //   m: the local median at (x,y) computed over the (kernel_size,kernel_size) neighborhood
    //   s: the local standard deviation at (x,y) computed over the (kernel_size,kernel_size) neighborhood
    //   k: a configurable parameter that weights the effect of the standard deviation
    //   R: the absolute maximum of the standard deviation
    //   T: the resulting threshold (everything below it will be white)
    //
    // The following code will try to make use of as few matrices as possible
    // as this speeds up the computation tremendously.

    cv::UMat threshold;

    {
        cv::UMat mean;
        cv::boxFilter(
            src,
            mean,
            CV_32F,
            cv::Size(kernel_size, kernel_size),
            cv::Point(-1, -1),
            true,
            cv::BORDER_REPLICATE
        );

        {
            cv::UMat sqmean;
            cv::sqrBoxFilter(
                src,
                sqmean,
                CV_32F,
                cv::Size(kernel_size, kernel_size),
                cv::Point(-1, -1),
                true,
                cv::BORDER_REPLICATE
            );

            cv::multiply(mean, mean, threshold); // -> mean²
            cv::subtract(sqmean, threshold, threshold); // sqmean - mean² -> variance
        }

        // √variance -> standard deviation (i.e. `s(x,y)`)
        cv::sqrt(threshold, threshold);

        // We now need to calculate:
        //    1 + k * ((s(x,y) / R) - 1)
        // We can help speed things up by inlining the multiplication of `k` like so:
        //    1 + ((s(x,y) * k / R) - k)
        // -> (s(x,y) * k / R) + 1 - k
        // As `k / R` and `1 - k` are constants (let's call them `a` and `b` respectively),
        // we can optimize this using a single fused-multiply-add step (cv::Mat::convertTo()):
        // -> s(x,y) * a + b
        threshold.convertTo(threshold, -1, k / 128.0, 1.0 - k);

        // We now got all parts ready and can finally compute:
        //   T = m(x,y) * ...
        cv::multiply(mean, threshold, threshold);
    }

    threshold.convertTo(threshold, src.depth());
    cv::compare(src, threshold, dst, threshold_type == cv::THRESH_BINARY ? cv::CMP_GT : cv::CMP_LE);

    trace.stop_and_write(dst);
}

} // namespace lhecker_bachelor::binarize
