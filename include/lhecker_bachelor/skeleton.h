#pragma once

#include <opencv2/core/mat.hpp>

namespace lhecker_bachelor::skeleton {

/*
 * NOTE: This method only works if `src` is a binary image, with 0 for black and 255 for white pixels.
 *
 * @article{DBLP:journals/cacm/ZhangS84,
 *     author = {T. Y. Zhang and Ching Y. Suen},
 *     title = {A Fast Parallel Algorithm for Thinning Digital Patterns},
 *     journal = {Commun. {ACM}},
 *     volume = {27},
 *     number = {3},
 *     pages = {236--239},
 *     year = {1984},
 *     url = {https://doi.org/10.1145/357994.358023},
 *     doi = {10.1145/357994.358023},
 *     timestamp = {Tue, 06 Nov 2018 12:51:40 +0100},
 *     biburl = {https://dblp.org/rec/bib/journals/cacm/ZhangS84},
 *     bibsource = {dblp computer science bibliography, https://dblp.org}
 * }
 */
void zhangsuen(cv::InputArray src, cv::OutputArray dst);

/*
 * NOTE: This method only works if `src` is a binary image, with 0 for black and 255 for white pixels.
 *
 * @inproceedings{DBLP:conf/icpr/ZhangW88a,
 *     author = {Edward Y. Y. Zhang and Patrick S. P. Wang},
 *     title = {A modified parallel thinning algorithm},
 *     booktitle = {9th International Conference on Pattern Recognition, {ICPR} 1988, 14-17 November 1988, Ergife Palace Hotel, Rome, Italy},
 *     pages = {1023--1025},
 *     year = {1988},
 *     crossref = {DBLP:conf/icpr/1988},
 *     url = {https://doi.org/10.1109/ICPR.1988.28429},
 *     doi = {10.1109/ICPR.1988.28429},
 *     timestamp = {Fri, 17 Nov 2017 16:56:00 +0100},
 *     biburl = {https://dblp.org/rec/bib/conf/icpr/ZhangW88a},
 *     bibsource = {dblp computer science bibliography, https://dblp.org}
 * }
 */
void zhangwang(cv::InputArray src, cv::OutputArray dst);

} // namespace lhecker_bachelor::skeleton
