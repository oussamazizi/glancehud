/// @file Snapshot.hpp
/// @brief Immutable, lock-free-publishable view of all current metric values.
#ifndef GLANCEHUD_SNAPSHOT_HPP
#define GLANCEHUD_SNAPSHOT_HPP

#include <array>
#include <cstdint>

#include "glancehud/MetricSample.hpp"
#include "glancehud/MetricType.hpp"

namespace glance {

/// @brief A consistent set of the latest values for every metric.
///
/// The engine builds a @ref Snapshot on the sampler thread and publishes it
/// atomically; renderers read it without locking. Fixed-size and trivially
/// copyable so no allocation happens on the hot path.
struct Snapshot {
    /// @brief Latest sample per metric, indexed by @c static_cast<size_t>(MetricType).
    std::array<MetricSample, kMetricTypeCount> samples{};

    /// @brief Monotonic counter incremented on each publish (drop detection / tests).
    std::uint64_t sequence{0};

    /// @brief Convenience accessor for a metric's latest sample.
    [[nodiscard]] const MetricSample& operator[](MetricType type) const noexcept {
        return samples[static_cast<std::size_t>(type)];
    }

    /// @brief Mutable accessor used by the engine while assembling a snapshot.
    [[nodiscard]] MetricSample& operator[](MetricType type) noexcept {
        return samples[static_cast<std::size_t>(type)];
    }
};

}  // namespace glance

#endif  // GLANCEHUD_SNAPSHOT_HPP
