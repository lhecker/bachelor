#pragma once

#include <opencv2/core/types.hpp>

namespace lhecker_bachelor::path {

class point {
public:
    constexpr point() = default;
    constexpr point(int32_t x, int32_t y, float radius) : x(x), y(y), radius(radius) {}

    template<typename T>
    constexpr point(cv::Point_<T> p, float radius) :
        x(cv::saturate_cast<int32_t>(p.x)),
        y(cv::saturate_cast<int32_t>(p.y)),
        radius(radius) {}

    [[nodiscard]] constexpr bool equals_locus(const point& other) const {
        return x == other.x && y == other.y;
    }

    template<typename T>
    explicit operator cv::Point_<T>() const noexcept {
        return {
            cv::saturate_cast<T>(x),
            cv::saturate_cast<T>(y),
        };
    }

    int32_t x = 0;
    int32_t y = 0;
    float radius = 0;
};

static_assert(std::is_trivially_copyable<point>::value);

class set {
    using container = std::vector<std::vector<point>>;

public:
    using value_type = container::value_type;
    using size_type = container::size_type;
    using difference_type = container::difference_type;
    using reference = container::reference;
    using const_reference = container::const_reference;
    using pointer = container::pointer;
    using const_pointer = container::const_pointer;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;
    using reverse_iterator = container::reverse_iterator;
    using const_reverse_iterator = container::const_reverse_iterator;

    // NOTE: This method only works if `skeleton` is a binary image, with 0 for black and 255 for white pixels.
    static set search(const cv::Mat& skeleton, const cv::Mat& distance);

    template<typename... Args>
    explicit constexpr set(Args&& ... args) : m_paths(std::forward<Args>(args)...) {}
    set(std::initializer_list<std::vector<point>> l) : m_paths(l.begin(), l.end()) {}

    void apply_approximation(double relative_error_bound);
    [[nodiscard]] std::string generate_svg(cv::Size2i size) const;

    [[nodiscard]] bool empty() const noexcept { return m_paths.empty(); }
    [[nodiscard]] size_type size() const noexcept { return m_paths.size(); }

    [[nodiscard]] const_iterator begin() const noexcept { return m_paths.begin(); }
    iterator end() noexcept { return m_paths.end(); }
    [[nodiscard]] const_iterator end() const noexcept { return m_paths.end(); }
    reverse_iterator rbegin() noexcept { return m_paths.rbegin(); }
    [[nodiscard]] const_reverse_iterator rbegin() const noexcept { return m_paths.rbegin(); }
    reverse_iterator rend() noexcept { return m_paths.rend(); }
    [[nodiscard]] const_reverse_iterator rend() const noexcept { return m_paths.rend(); }
    [[nodiscard]] const_iterator cbegin() const noexcept { return m_paths.cbegin(); }
    [[nodiscard]] const_iterator cend() const noexcept { return m_paths.cend(); }
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept { return m_paths.crbegin(); }
    [[nodiscard]] const_reverse_iterator crend() const noexcept { return m_paths.crend(); }

private:
    container m_paths;
};

} // namespace lhecker_bachelor::path
