#include <lhecker_bachelor/path.h>

#include <iostream>

#include <lhecker_bachelor/tracer.h>

namespace lhecker_bachelor::path {

static cv::Point2d line_normal(cv::Point2d p1, cv::Point2d p2) {
    // Given the directional vector d of the line between p1 and p2, calculated using (p1 - p2),
    // the 2 normals are calculated using (-d.y, d.x) or (d.y, -d.x).
    // The code below is an inlined version of that.
    auto d = cv::Point2d{p2.y - p1.y, p1.x - p2.x};
    return d / cv::norm(d);
}

static void operator+=(std::string& str, int num) {
    std::array<char, 16> buffer; // NOLINT

    const auto n = std::snprintf(buffer.data(), buffer.size(), "%i", num);
    if (n < 0 || std::size_t(n) > buffer.size()) {
        throw std::runtime_error("failed to format int");
    }

    str.append(std::string_view{buffer.data(), std::size_t(n)});
}

static void operator+=(std::string& str, double num) {
    std::array<char, 16> buffer; // NOLINT

    const auto n = std::snprintf(buffer.data(), buffer.size(), "%.3f", num);
    if (n < 0 || std::size_t(n) > buffer.size()) {
        throw std::runtime_error("failed to format double");
    }

    str.append(std::string_view{buffer.data(), std::size_t(n)});
}

static void operator+=(std::string& str, cv::Point2d p) {
    str += p.x;
    str += ',';
    str += p.y;
}

static void operator+=(std::string& str, point p) {
    str += p.x;
    str += ',';
    str += p.y;
}

class point_series_builder {
public:
    explicit point_series_builder(std::string& target) : m_target(target) {}

    point_series_builder& write(cv::Point2d p) {
        if (m_seperator_needed) {
            m_target += ' ';
        }

        m_target += p;
        m_seperator_needed = true;

        return *this;
    }

    point_series_builder& write(point p) {
        if (m_seperator_needed) {
            m_target += ' ';
        }

        m_target += p;
        m_seperator_needed = true;

        return *this;
    }

private:
    std::string& m_target;
    bool m_seperator_needed = false;
};

std::string set::generate_svg(cv::Size2i size) const {
    tracer trace{"lhecker_bachelor::path::set::generate_svg"};

    std::string dst;
    dst.reserve(64 * 1024);

    dst += R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 )";
    dst += size.width;
    dst += ' ';
    dst += size.height;
    dst += R"(" width=")";
    dst += size.width;
    dst += R"(" height=")";
    dst += size.height;
    dst += R"#(">
<defs>
    <marker id="dot" viewBox="0 0 10 10" refX="5" refY="5" markerWidth="5" markerHeight="5">
        <circle cx="5" cy="5" r="5" fill="white" stroke="black"/>
    </marker>
</defs>
<g>
)#";

    {
        std::size_t idx = 0;

        for (const auto& path : m_paths) {
            dst += "    <g id=\"p";
            dst += int(idx);
            dst += "\">\n";

            for (auto ptr = path.data(), end = ptr + path.size() - 1; ptr < end; ++ptr) {
                point waypoint0 = ptr[0];
                point waypoint1 = ptr[1];

                cv::Point2d point0(waypoint0);
                cv::Point2d point1(waypoint1);

                auto normal = line_normal(point1, point0);
                auto normal0 = normal * waypoint0.radius;
                auto normal1 = normal * waypoint1.radius;

                dst += "        <polygon points=\"";
                point_series_builder(dst)
                    .write(point0 - normal0)
                    .write(point0 + normal0)
                    .write(point1 + normal1)
                    .write(point1 - normal1);
                dst += "\"/>\n";
            }

            for (auto p : path) {
                dst += "        <circle cx=\"";
                dst += p.x;
                dst += "\" cy=\"";
                dst += p.y;
                dst += "\" r=\"";
                dst += p.radius;
                dst += "\"/>\n";
            }

            dst += "    </g>\n";

            idx++;
        }
    }

    dst += R"#(</g>
<g visibility="hidden" pointer-events="none" fill="none" stroke="white" stroke-width="0.5" marker-start="url(#dot)" marker-mid="url(#dot)" marker-end="url(#dot)">
)#";

    {
        std::size_t idx = 0;

        for (const auto& path : m_paths) {
            point_series_builder builder(dst);

            dst += "    <polyline id=\"c";
            dst += int(idx);
            dst += "\" points=\"";

            for (auto p : path) {
                builder.write(p);
            }

            dst += "\"/>\n";

            idx++;
        }
    }

    dst += R"#(</g>
<script><![CDATA[(() => {
    let visibleElement;
    document.addEventListener("mouseover", e => {
        if (visibleElement) {
            visibleElement.style.visibility = "";
            visibleElement = undefined;
        }

        const parent = e.target.parentElement;
        if (!parent) {
            return;
        }

        const path = "c" + parent.id.slice(1);
        if (!path) {
            return;
        }

        visibleElement = document.getElementById(path);
        visibleElement.style.visibility = "visible";
    });

    function onkeydown(e) {
        if (e.key !== "c" || e.shiftKey || e.ctrlKey || e.altKey) {
            return;
        }

        document.removeEventListener("keydown", onkeydown)
        document.documentElement.style.backgroundColor = "#222";
        Array.from(document.querySelectorAll('g[id^="p"]')).forEach((e, i) => {
            e.style.fill = `hsla(${(i * 47) % 360}, 100%, 50%, 50%)`;
        });

        event.preventDefault();
    }

    document.addEventListener("keydown", onkeydown);
})()]]></script>
</svg>
)#";

    return dst;
}

} // namespace lhecker_bachelor::path
