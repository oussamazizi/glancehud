/// @file IMetricProvider.hpp
/// @brief Contract implemented by platform-specific metric sources.
#ifndef GLANCEHUD_IMETRIC_PROVIDER_HPP
#define GLANCEHUD_IMETRIC_PROVIDER_HPP

#include "glancehud/MetricSample.hpp"
#include "glancehud/MetricType.hpp"

namespace glance {

/// @brief Produces samples for one metric.
///
/// The core owns no platform code: Android (JNI) and iOS (Obj-C++) implement
/// concrete providers (CPU, RAM, GPU, network) and register them with the
/// engine. This is the dependency-inversion boundary that keeps @c core pure.
///
/// @note Implementations must be cheap and non-blocking: @ref sample runs on the
///       sampler thread at the configured interval.
class IMetricProvider {
public:
    virtual ~IMetricProvider() = default;

    /// @brief The metric this provider is responsible for.
    [[nodiscard]] virtual MetricType type() const noexcept = 0;

    /// @brief Read the metric now. Set @c MetricSample::valid to false if
    ///        the value cannot be obtained on this platform/frame.
    [[nodiscard]] virtual MetricSample sample(MetricSample::TimePoint now) = 0;
};

}  // namespace glance

#endif  // GLANCEHUD_IMETRIC_PROVIDER_HPP
