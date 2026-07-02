/// @file CpuProvider.hpp
/// @brief Android/Linux CPU usage provider (reads /proc).
#ifndef GLANCEHUD_ANDROID_CPU_PROVIDER_HPP
#define GLANCEHUD_ANDROID_CPU_PROVIDER_HPP

#include <cstdint>

#include "glancehud/IMetricProvider.hpp"

namespace glance::android {

/// @brief Reports this process's CPU usage as a percentage of one core.
///
/// Computed from the deltas of process jiffies (/proc/self/stat) versus total
/// system jiffies (/proc/stat) between two samples. The very first sample has no
/// previous reference, so it is reported as invalid.
///
/// This class is Android/Linux-specific (it relies on /proc) and therefore lives
/// in the platform module, never in the pure core.
class CpuProvider final : public IMetricProvider {
public:
    [[nodiscard]] MetricType type() const noexcept override { return MetricType::CpuUsage; }
    [[nodiscard]] MetricSample sample(MetricSample::TimePoint now) override;

private:
    std::uint64_t lastProcess_{0};  ///< Previous process jiffies (utime+stime).
    std::uint64_t lastTotal_{0};    ///< Previous total system jiffies.
    bool havePrevious_{false};
};

}  // namespace glance::android

#endif  // GLANCEHUD_ANDROID_CPU_PROVIDER_HPP
