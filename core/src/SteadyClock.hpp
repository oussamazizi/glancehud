/// @file SteadyClock.hpp
/// @brief Default @ref glance::IClock backed by std::chrono::steady_clock.
///
/// Internal to the core (not part of the public API). Tests inject a fake clock
/// instead so timing-dependent logic is deterministic.
#ifndef GLANCEHUD_SRC_STEADY_CLOCK_HPP
#define GLANCEHUD_SRC_STEADY_CLOCK_HPP

#include <chrono>

#include "glancehud/IClock.hpp"

namespace glance {

/// @brief Real monotonic clock used in production.
class SteadyClock final : public IClock {
public:
    [[nodiscard]] MetricSample::TimePoint now() const noexcept override {
        return std::chrono::steady_clock::now();
    }
};

}  // namespace glance

#endif  // GLANCEHUD_SRC_STEADY_CLOCK_HPP
