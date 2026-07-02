/// @file FpsCalculator.hpp
/// @brief Derives FPS and frame time from host frame timestamps.
#ifndef GLANCEHUD_SRC_FPS_CALCULATOR_HPP
#define GLANCEHUD_SRC_FPS_CALCULATOR_HPP

#include <atomic>

#include "glancehud/MetricSample.hpp"

namespace glance {

/// @brief Computes smoothed FPS and instantaneous frame time.
///
/// @ref onFrame is called from the host's frame callback (UI thread); the
/// getters are read from the sampler thread. The published results are kept in
/// atomics so this cross-thread hand-off needs no lock and no allocation.
///
/// FPS is smoothed with an exponential moving average (EMA) so the displayed
/// value is stable rather than jittering every frame.
class FpsCalculator {
public:
    /// @brief Register a frame at time @p now (UI thread).
    void onFrame(MetricSample::TimePoint now) noexcept;

    /// @brief Clear accumulated state (e.g. on start).
    void reset() noexcept;

    /// @brief True once at least two frames have been seen.
    [[nodiscard]] bool hasData() const noexcept { return hasData_.load(std::memory_order_relaxed); }

    /// @brief Smoothed frames per second.
    [[nodiscard]] double fps() const noexcept { return fps_.load(std::memory_order_relaxed); }

    /// @brief Duration of the most recent frame, in milliseconds.
    [[nodiscard]] double frameTimeMs() const noexcept {
        return frameTimeMs_.load(std::memory_order_relaxed);
    }

private:
    /// Smoothing factor for the EMA: higher = more responsive, less smooth.
    static constexpr double kEmaAlpha = 0.1;

    // Touched only on the UI thread (inside onFrame):
    MetricSample::TimePoint last_{};
    bool haveLast_{false};
    double emaMs_{0.0};
    bool emaInitialised_{false};

    // Published results, read from other threads:
    std::atomic<bool> hasData_{false};
    std::atomic<double> frameTimeMs_{0.0};
    std::atomic<double> fps_{0.0};
};

}  // namespace glance

#endif  // GLANCEHUD_SRC_FPS_CALCULATOR_HPP
