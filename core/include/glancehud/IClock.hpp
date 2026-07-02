/// @file IClock.hpp
/// @brief Abstraction over the time source, injectable for deterministic tests.
#ifndef GLANCEHUD_ICLOCK_HPP
#define GLANCEHUD_ICLOCK_HPP

#include "glancehud/MetricSample.hpp"

namespace glance {

/// @brief Interface for reading the current time.
///
/// Injecting the clock lets the FPS/jank logic be tested deterministically with
/// a fake clock instead of wall-clock timing.
class IClock {
public:
    virtual ~IClock() = default;

    /// @brief The current steady-clock time point.
    [[nodiscard]] virtual MetricSample::TimePoint now() const noexcept = 0;
};

}  // namespace glance

#endif  // GLANCEHUD_ICLOCK_HPP
