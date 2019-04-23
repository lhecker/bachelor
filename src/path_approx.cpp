#include <lhecker_bachelor/path.h>

#include <numeric>
#include <stack>

#include <lhecker_bachelor/tracer.h>

namespace lhecker_bachelor::path {

constexpr void invalidate_point(point& point) {
    point.radius = 0;
}

constexpr bool is_invalid_point(const point& point) {
    return point.radius == 0;
}

static double euclidian_distance(const point& a, const point& b) {
    auto d = cv::Point2d(a) - cv::Point2d(b);
    return d.x * d.x + d.y * d.y;
}

struct error_calculator {
    error_calculator(const point& beg, const point& end) : _beg(beg), _beg_radius(beg.radius) {
        auto end_beg_delta = cv::Point2d(end) - this->_beg;
        _end_beg_distance = cv::norm(end_beg_delta);
        _end_beg_normal = end_beg_delta / _end_beg_distance;
        _end_beg_radius_delta = double(end.radius) - _beg_radius;
    }

    double relative_error(const point& pt) {
        // An example…
        // pt with it's radius protrudes significantly outside of the isosceles trapezoid, beg/end and their radii form.
        //
        //                 ┌─p─┐
        //               ┌─i─┐
        //               ┌d┐
        //   end →     ( ● )
        //            ╱  │  ╲
        //   pt  →   ╱ ( x─● ╲ )
        //          ╱    │    ╲
        //   beg → (     ●     )
        //
        // x: the "intersection"
        // d: the displacement of pt from the beg/end line (displacement)
        // p: the radius of pt (pt.radius)
        // i: the radius of the "intersection" if beg/end was a straight line (interpolated_radius)

        // Using the dot product we can calculate how far along on the vector (end - beg) the intersection "x" would be.
        // The coordinate of the intersection is then easy to calculate.
        // See: https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Vector_formulation
        auto pt_delta = cv::Point2d(pt) - _beg;
        auto intersection_distance = _end_beg_normal.ddot(pt_delta);
        auto intersection_coord = intersection_distance * _end_beg_normal;
        auto pt_intersection_delta = pt_delta - intersection_coord;
        auto displacement = cv::norm(pt_intersection_delta);

        // intersection_rel_pos is within the range (0;1) and contains the relative position of pt between beg and end.
        // Using this we can interpolate what the radius would be if pt wouldn't exist.
        auto intersection_rel_pos = intersection_distance / _end_beg_distance;
        auto interpolated_radius = _beg_radius + intersection_rel_pos * _end_beg_radius_delta;
        auto radius_delta = std::abs(interpolated_radius - double(pt.radius));

        return std::max(radius_delta, displacement);
    }

private:
    cv::Point2d _beg;
    double _beg_radius;
    cv::Point2d _end_beg_normal;
    double _end_beg_radius_delta;
    double _end_beg_distance;
};

static void path_apply_approximation(
    std::vector<point>& src_contour,
    double relative_error_bound
) {
    const auto src_size = src_contour.size();
    if (src_size < 2) {
        return;
    }

    const auto& src_contour_front = src_contour.front();
    const auto& src_contour_back = src_contour.back();
    const bool is_cycle = src_contour_front.equals_locus(src_contour_back);

    // Nothing to simplify here if there are only to points anyways.
    if (src_size == 2) {
        // Although we can remove one of the two if it's a cycle with just 2 elements.
        // (Theoretically this will never happen, but better safe than sorry.)
        if (is_cycle) {
            src_contour.pop_back();
        }
        return;
    }

    std::stack<std::array<std::size_t, 2>> stack;

    // The simplification will have to calculate the distance between first and last point in the first iteration.
    // If this line path is a cycle though, the distance will be 0 and the calculation of the dot product incorrect.
    // Due to that we have to split up cycles in 2 parts beforehand.
    if (is_cycle) {
        double max_distance = 0.0;
        std::size_t index = 1;

        for (std::size_t i = 1; i < src_size - 1; i++) {
            double d = euclidian_distance(src_contour_front, src_contour[i]);
            if (d > max_distance) {
                max_distance = d;
                index = i;
            }
        }

        stack.push({{0, index}});
        stack.push({{index, src_size - 1}});
    } else {
        stack.push({{0, src_size - 1}});
    }

    // The remaining part is an entirely classical, iterative implementation of the Douglas-Peucker algorithm.
    while (!stack.empty()) {
        auto[index_beg, index_end] = stack.top();
        stack.pop();

        error_calculator calc{src_contour[index_beg], src_contour[index_end]};
        double max_relative_error = 0.0;
        std::size_t index = 0;

        for (std::size_t i = index_beg + 1; i < index_end; i++) {
            const auto point = src_contour[i];
            if (is_invalid_point(point)) {
                continue;
            }

            const auto e = calc.relative_error(point);
            if (e > max_relative_error) {
                max_relative_error = e;
                index = i;
            }
        }

        if (max_relative_error > relative_error_bound) {
            stack.push({{index_beg, index}});
            stack.push({{index, index_end}});
            continue;
        }

        for (std::size_t i = index_beg + 1; i < index_end; i++) {
            invalidate_point(src_contour[i]);
        }
    }

    // Erase–remove idiom: https://en.wikipedia.org/wiki/Erase-remove_idiom
    src_contour.erase(std::remove_if(src_contour.begin(), src_contour.end(), is_invalid_point), src_contour.end());
}

void set::apply_approximation(double relative_error_bound) {
    tracer trace{"lhecker_bachelor::path::set::apply_approximation"};

    // Other methods in this file operate with halfed errrors.
    relative_error_bound /= 2.0;

    for (auto& path : m_paths) {
        path_apply_approximation(path, relative_error_bound);
    }
}

} // namespace lhecker_bachelor::path
