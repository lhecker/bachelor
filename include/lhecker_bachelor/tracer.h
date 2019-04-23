#pragma once

#include <functional>
#include <string_view>

#include <opencv2/core/mat.hpp>

namespace lhecker_bachelor {

class tracer {
public:
    enum class level : uint8_t {
        disabled = 0b00,
        timings = 0b01,
        snapshots = 0b10,
    };

    struct log_entry {
        std::string_view title;
        double millis;
    };

    // The following functions are not thread-safe and it's expected
    // that you call them at the very start of your program.
    static void set_level(level level) noexcept;
    static void set_log_handler(std::function<void(const log_entry&)> handler) noexcept;

    static inline bool has_level(level level) noexcept;

    explicit tracer(std::string_view title) noexcept;

    tracer(const tracer& other) = delete;
    tracer(tracer&& other) = default;

    tracer& operator=(const tracer& other) = delete;
    tracer& operator=(tracer&& other) = default;

    ~tracer();

    void stop() noexcept;
    void stop_and_write(cv::InputArray image);

private:
    static void delete_out_files();

    static level m_level;
    static std::function<void(const log_entry&)> m_log_handler;

    std::string_view m_title;
    int64_t m_beg = 0;
};

constexpr tracer::level operator|(tracer::level a, tracer::level b) {
    return static_cast<tracer::level>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr tracer::level operator&(tracer::level a, tracer::level b) {
    return static_cast<tracer::level>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline bool tracer::has_level(tracer::level level) noexcept {
    return (m_level & level) == level;
}

} // namespace lhecker_bachelor
