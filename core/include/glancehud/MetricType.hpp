/// @file MetricType.hpp
/// @brief Enumeration of the performance metrics tracked by GlanceHUD.
#ifndef GLANCEHUD_METRIC_TYPE_HPP
#define GLANCEHUD_METRIC_TYPE_HPP

#include <cstdint>
#include <string_view>

namespace glance {

/// @brief The kinds of metric a provider can produce.
///
/// The enum is stable and ordered; @ref MetricType::Count must remain last so
/// it can size fixed arrays without heap allocation.
enum class MetricType : std::uint8_t {
    Fps = 0,        ///< Frames per second (Hz).
    FrameTime,      ///< Time to produce one frame (ms).
    CpuUsage,       ///< Process CPU usage (percent, 0..100+).
    RamUsage,       ///< Resident memory of the process (MB).
    GpuTime,        ///< GPU time per frame (ms), when available.
    NetUpload,      ///< Upload throughput (KB/s).
    NetDownload,    ///< Download throughput (KB/s).
    Count           ///< Sentinel: number of metric types. Keep last.
};

/// @brief Number of distinct metric types (excludes the sentinel).
inline constexpr std::size_t kMetricTypeCount = static_cast<std::size_t>(MetricType::Count);

/// @brief Human-readable short label for a metric (e.g. "FPS").
[[nodiscard]] constexpr std::string_view toLabel(MetricType type) noexcept {
    switch (type) {
        case MetricType::Fps:         return "FPS";
        case MetricType::FrameTime:   return "Frame";
        case MetricType::CpuUsage:    return "CPU";
        case MetricType::RamUsage:    return "RAM";
        case MetricType::GpuTime:     return "GPU";
        case MetricType::NetUpload:   return "NET UP";
        case MetricType::NetDownload: return "NET DOWN";
        case MetricType::Count:       return "";
    }
    return "";
}

/// @brief Canonical unit for a metric (e.g. "ms", "MB", "KB/s").
[[nodiscard]] constexpr std::string_view toUnit(MetricType type) noexcept {
    switch (type) {
        case MetricType::Fps:         return "";
        case MetricType::FrameTime:   return "ms";
        case MetricType::CpuUsage:    return "%";
        case MetricType::RamUsage:    return "MB";
        case MetricType::GpuTime:     return "ms";
        case MetricType::NetUpload:   return "KB/s";
        case MetricType::NetDownload: return "KB/s";
        case MetricType::Count:       return "";
    }
    return "";
}

}  // namespace glance

#endif  // GLANCEHUD_METRIC_TYPE_HPP
