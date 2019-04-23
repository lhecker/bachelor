#pragma once

namespace detail {

template<typename F>
class scope_guard {
public:
    scope_guard(F f) noexcept : func(std::move(f)) {
    }

    scope_guard(const scope_guard&) = delete;
    scope_guard& operator=(const scope_guard&) = delete;
    scope_guard& operator=(scope_guard&&) = delete;

    scope_guard(scope_guard&& rhs) noexcept : func(std::move(rhs.func)), present(rhs.present) {
        rhs.present = false;
    }

    ~scope_guard() {
        if (present) {
            func();
        }
    }

private:
    F func;
    bool present = true;
};

enum class scope_guard_helper {
};

template<typename F>
scope_guard<F> operator+(scope_guard_helper /*unused*/, F&& fn) {
    return scope_guard<F>(std::forward<F>(fn));
}

} // namespace detail

// The extra indirection is necessary to prevent __LINE__ to be treated literally.
#define _DEFER_CONCAT_IMPL(a, b) a ## b
#define _DEFER_CONCAT(a, b) _DEFER_CONCAT_IMPL(a, b)

#define defer const auto _DEFER_CONCAT(_defer_, __LINE__) = ::detail::scope_guard_helper() + [&]()
