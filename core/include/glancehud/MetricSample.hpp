/// @file MetricSample.hpp
/// @brief A single timestamped measurement produced by a metric provider.
#ifndef GLANCEHUD_METRIC_SAMPLE_HPP
#define GLANCEHUD_METRIC_SAMPLE_HPP

#include <chrono>

#include "glancehud/MetricType.hpp"

namespace glance {

/// @brief One immutable measurement of a single @ref MetricType.
///
/// Trivially copyable and allocation-free by design: samples flow between the
/// sampler thread and the render thread without heap traffic.
struct MetricSample {
    /// @brief Steady-clock timestamp for time-base independent computations.
    using TimePoint = std::chrono::steady_clock::time_point;

    MetricType type{MetricType::Count};  ///< Which metric this sample measures.
    double value{0.0};                   ///< Value in the metric's canonical unit.
    TimePoint timestamp{};               ///< When the sample was taken.
    bool valid{false};                   ///< False when the metric is unavailable.
};

}  // namespace glance

#endif  // GLANCEHUD_METRIC_SAMPLE_HPP
