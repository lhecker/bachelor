#include <lhecker_bachelor/path.h>

#include <utility>

#include <lhecker_bachelor/tracer.h>

struct point_delta {
    [[nodiscard]] constexpr bool non_null() const noexcept {
        return x != 0 || y != 0;
    }

    int8_t x = 0;
    int8_t y = 0;
};

inline cv::Point2i operator+(cv::Point2i p, point_delta d) noexcept {
    return {p.x + d.x, p.y + d.y};
}

enum class claim_state : uint8_t {
    none = 0,
    path,
    junction,
};

namespace cv {

template<>
class DataType<claim_state> : public DataType<uint8_t> {
};

} // namespace cv

struct neighbor_search_result {
    neighbor_search_result() = default;

    neighbor_search_result(
        cv::Point2i neighbor,
        point_delta direction,
        claim_state claim
    ) : neighbor(neighbor),
        direction(direction),
        claim(claim) {}

    static neighbor_search_result with_center_and_direction(
        cv::Point2i center,
        point_delta direction,
        claim_state claim
    ) {
        return neighbor_search_result(center + direction, direction, claim);
    }

    cv::Point2i neighbor;
    point_delta direction;
    claim_state claim = claim_state::none;
};

// It's important that horizontal/vertical directions are ordered first.
// This ensures that "steps" at the start of a path are handled correctly.
// I.e. lines like these:
//    ■■■
//   ■■
// It's important it's not parsed like this:
//    →→→
//   ↗
// But like this:
//    →→→
//   →↑
static const std::array<point_delta, 8> initial_neighborhood{{
    {0, -1},  // ↑
    {1, 0},   // →
    {0, 1},   // ↓
    {-1, 0},  // ←
    {-1, -1}, // ↖
    {1, -1},  // ↗
    {1, 1},   // ↘
    {-1, 1},  // ↙
}};

constexpr size_t neighborhood_loop_length = 5;

static const std::array<point_delta, 12> neighborhood_loop{{
    {-1, -1}, //  0 ↖
    {0, -1},  //  1 ↑
    {1, -1},  //  2 ↗
    {1, 0},   //  3 →
    {1, 1},   //  4 ↘
    {0, 1},   //  5 ↓
    {-1, 1},  //  6 ↙
    {-1, 0},  //  7 ←
    {-1, -1}, //  8 ↖
    {0, -1},  //  9 ↑
    {1, -1},  // 10 ↗
    {1, 0},   // 11 →
}};

// This array contains offsets for indexing into neighborhood_loop above.
// Each find_unclaimed_neighbor_at() call will iterate through 5 neighborhood_loop entries, starting from this offset.
static const std::array<std::array<uint8_t, 3>, 3> neighborhood_loop_offset{{
    {{
        // ↖↑↗
        // ←●
        // ↙
        6,
        // ↖↑↗
        // ←●→
        //
        7,
        // ↖↑↗
        //  ●→
        //   ↘
        0,
    }},
    {{
        // ↖↑
        // ←●
        // ↙↓
        5,
        //
        //  ●
        //
        0,
        //  ↑↗
        //  ●→
        //  ↓↘
        1,
    }},
    {{
        // ↖
        // ←●
        // ↙↓↘
        4,
        //
        // ←●→
        // ↙↓↘
        3,
        //   ↗
        //  ●→
        // ↙↓↘
        2,
    }},
}};

namespace lhecker_bachelor::path {

static neighbor_search_result find_unclaimed_neighbor_at(
    const cv::Mat& skeleton,
    const cv::Mat& claimed,
    cv::Point2i center,
    const point_delta direction
) {
    // This function has a ambiguity check quite similar to Zhang-Suen's thinning algorithm.
    // The state is considered "ambiguous" if we face neighbors like these:
    // ←●→
    //   ↘
    enum class state_t {
        uncommitted = 0,
        committed,
        ambigious
    };

    const auto offset = neighborhood_loop_offset[direction.y + 1][direction.x + 1];
    const auto neighbors = neighborhood_loop.data() + offset;

    std::array<point_delta, neighborhood_loop_length> found;
    int num_found = 0;
    auto state = state_t::uncommitted;

    for (std::size_t i = 0; i < neighborhood_loop_length; ++i) {
        const auto d = neighbors[i];
        const auto p = center + d;

        const auto inside = p.x >= 0 && p.x < skeleton.cols && p.y >= 0 && p.y < skeleton.rows;
        const auto value = inside ? skeleton.at<uint8_t>(p) : 0;
        if (value == 0) {
            if (num_found != 0 && state == state_t::uncommitted) {
                state = state_t::committed;
            }
            continue;
        }

        auto claim = claimed.at<claim_state>(p);
        switch (claim) {
            case claim_state::none:
                break;
            case claim_state::path:
                continue;
            case claim_state::junction:
                // One of our neighbors is an junction => connect to it
                return neighbor_search_result{
                    p,
                    d,
                    claim_state::junction,
                };
        }

        found[num_found] = d;
        ++num_found;

        if (state == state_t::committed) {
            state = state_t::ambigious;
        }
    }

    // 2+ disjoint neighbors => we're an junction
    // E.g.:
    // ←●→
    //   ↘
    // => Choose ● and mark it as an junction.
    if (state == state_t::ambigious) {
        return neighbor_search_result{
            center,
            {},
            claim_state::junction,
        };
    }

    // Due to the check above all found neighbors will adjacent to each other.
    // E.g.:
    //   ↗
    //  ●→
    //   ↘
    // (num_found will be 3 in this case.)

    switch (num_found) {
        case 1:
            // E.g.:
            //  ●→
            // => Choose → and proceed.
            return neighbor_search_result::with_center_and_direction(
                center,
                found[0],
                claim_state::path
            );
        case 2: {
            // E.g.:
            //  ●→
            //   ↘
            // => Prefer horizontal/vertical directions over diagonal ones.
            // => Choose → and proceed.
            const auto idx = found[0].x == 0 || found[0].y == 0 ? 0 : 1;
            return neighbor_search_result::with_center_and_direction(
                center,
                found[idx],
                claim_state::path
            );
        }
        case 3:
            // E.g.:
            //   ↗
            //  ●→
            //   ↘
            // => Choose → and mark it as an junction.
            return neighbor_search_result::with_center_and_direction(
                center,
                found[1],
                claim_state::junction
            );
        default:
            // If 0 or more than 3 => return ::stop.
            // (Because I don't know whom to choose if there are >3 neighbors.)
            return {};
    }
}

static void find_paths_at(
    const cv::Mat& skeleton,
    const cv::Mat& distance,
    cv::Mat& claims,
    cv::Point2i root,
    std::vector<std::vector<point>>& paths
) {
    auto& root_claim_ref = claims.at<claim_state>(root);
    auto root_claim = root_claim_ref;

    switch (root_claim) {
        case claim_state::none:
            root_claim_ref = claim_state::junction;
            break;
        case claim_state::path:
            return;
        case claim_state::junction:
            break;
    }

    point root_waypoint{root, distance.at<float>(root)};
    bool neighbor_found = false;

    for (const auto& d : initial_neighborhood) {
        auto p = root + d;

        if (p.x < 0 || p.x >= skeleton.cols ||
            p.y < 0 || p.y >= skeleton.rows) {
            continue;
        }

        auto value = skeleton.at<uint8_t>(p);
        if (value == 0) {
            continue;
        }

        auto claim = claims.at<claim_state>(p);
        if (claim != claim_state::none) {
            continue;
        }

        // The initial neighbor will be claimed during the first iteration of the main loop below.
        neighbor_search_result result{p, d, claim_state::path};

        std::vector<point> path;
        path.push_back(root_waypoint);

        while (true) {
            // After find_unclaimed_neighbor_at() was called in the previous iteration,
            // result can be in 4 different states we need to be able to handle. Namely:
            // -
            //   * result.claim is "none"
            //   => stop the search
            // -
            //   * result.claim is "path"
            //   * the result.neighbor did change
            //   => continue the search regularly
            // -
            //   * result.claim is "junction"
            //   * the result.neighbor did change
            //   => the new point was recognized as an junction
            //   => stop the search afterwards
            // -
            //   * result.claim is "junction"
            //   * the result.neighbor didn't change
            //   * thus result.direction is (0,0)
            //   => the previous (= new) point was recognized as an junction
            //   => stop the search here

            if (result.claim != claim_state::none) {
                claims.at<claim_state>(result.neighbor) = result.claim;

                if (result.direction.non_null()) {
                    path.emplace_back(
                        result.neighbor.x,
                        result.neighbor.y,
                        distance.at<float>(result.neighbor)
                    );
                }
            }

            if (result.claim != claim_state::path) {
                break;
            }

            result = find_unclaimed_neighbor_at(skeleton, claims, result.neighbor, result.direction);
        }

        paths.emplace_back(std::move(path));
        neighbor_found = true;
    }

    // If this is a single, alone dot no neighbors will be found, but we should still save it as a path.
    // We need to consider root_claim though, because we should not add this path, if we're just revisiting
    // a pixel that has been marked as a junction before. (In theory this cannot happen since the
    // pixel wouldn't have been marked as a junction then, but better safe than sorry.)
    if (!neighbor_found && root_claim == claim_state::none) {
        paths.emplace_back(std::vector<point>{root_waypoint});
    }
}

set set::search(const cv::Mat& skeleton, const cv::Mat& distance) {
    CV_Assert(skeleton.type() == CV_8UC1); // NOLINT
    CV_Assert(distance.type() == CV_32FC1); // NOLINT

    tracer trace{"lhecker_bachelor::path::set::search"};

    cv::Mat claims{skeleton.size(), CV_8UC1, cv::Scalar{}}; // NOLINT
    std::vector<std::vector<point>> paths;

    for (int row = 0; row < skeleton.rows; ++row) {
        const auto skeleton_row_beg = skeleton.ptr<uint8_t>(row);
        const auto skeleton_row_end = skeleton_row_beg + skeleton.cols;

        for (auto skeleton_p = skeleton_row_beg; skeleton_p != skeleton_row_end; skeleton_p++) {
            const auto remaining = skeleton_row_end - skeleton_p;

            skeleton_p = static_cast<const uint8_t*>(memchr(skeleton_p, 255, remaining));
            if (!skeleton_p) {
                break;
            }

            const auto col = int(skeleton_p - skeleton_row_beg);
            const cv::Point2i p(col, row);
            find_paths_at(skeleton, distance, claims, p, paths);
        }
    }

    return set(std::move(paths));
}

} // namespace lhecker_bachelor::path
