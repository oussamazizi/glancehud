/// @file RamProvider.hpp
/// @brief Android/Linux resident memory provider (reads /proc).
#ifndef GLANCEHUD_ANDROID_RAM_PROVIDER_HPP
#define GLANCEHUD_ANDROID_RAM_PROVIDER_HPP

#include "glancehud/IMetricProvider.hpp"

namespace glance::android {

/// @brief Reports the process's resident set size (RSS) in megabytes.
///
/// Reads /proc/self/statm (field 2 = resident pages) and multiplies by the page
/// size. Android/Linux-specific, hence in the platform module.
class RamProvider final : public IMetricProvider {
public:
    [[nodiscard]] MetricType type() const noexcept override { return MetricType::RamUsage; }
    [[nodiscard]] MetricSample sample(MetricSample::TimePoint now) override;
};

}  // namespace glance::android

#endif  // GLANCEHUD_ANDROID_RAM_PROVIDER_HPP
