#include "FpsCalculator.hpp"

#include <chrono>

namespace glance {

void FpsCalculator::onFrame(MetricSample::TimePoint now) noexcept {
    if (!haveLast_) {
        last_ = now;
        haveLast_ = true;
        return;
    }

    const std::chrono::duration<double, std::milli> delta = now - last_;
    last_ = now;

    const double deltaMs = delta.count();
    if (deltaMs <= 0.0) {
        return;  // Ignore duplicate / out-of-order timestamps.
    }

    frameTimeMs_.store(deltaMs, std::memory_order_relaxed);

    if (!emaInitialised_) {
        emaMs_ = deltaMs;
        emaInitialised_ = true;
    } else {
        emaMs_ = kEmaAlpha * deltaMs + (1.0 - kEmaAlpha) * emaMs_;
    }

    fps_.store(emaMs_ > 0.0 ? 1000.0 / emaMs_ : 0.0, std::memory_order_relaxed);
    hasData_.store(true, std::memory_order_relaxed);
}

void FpsCalculator::reset() noexcept {
    haveLast_ = false;
    emaInitialised_ = false;
    emaMs_ = 0.0;
    hasData_.store(false, std::memory_order_relaxed);
    frameTimeMs_.store(0.0, std::memory_order_relaxed);
    fps_.store(0.0, std::memory_order_relaxed);
}

}  // namespace glance
