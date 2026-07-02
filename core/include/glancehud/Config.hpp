/// @file Config.hpp
/// @brief User-tunable configuration for the overlay and sampler.
#ifndef GLANCEHUD_CONFIG_HPP
#define GLANCEHUD_CONFIG_HPP

#include <chrono>
#include <cstdint>

namespace glance {

/// @brief Anchor corner used for the initial overlay position.
enum class Anchor : std::uint8_t { TopLeft, TopRight, BottomLeft, BottomRight };

/// @brief Immutable configuration passed at @c PerfOverlay::initialize().
struct Config {
    /// @brief How often non-frame metrics (CPU/RAM/net) are sampled.
    std::chrono::milliseconds sampleInterval{500};

    /// @brief Overlay opacity in [0, 1].
    float opacity{0.85f};

    /// @brief Initial anchor corner.
    Anchor anchor{Anchor::TopRight};

    /// @brief Start collapsed (only a small handle shown).
    bool startCollapsed{false};

    /// @brief Show the overlay immediately after @c start().
    bool autoShow{true};
};

}  // namespace glance

#endif  // GLANCEHUD_CONFIG_HPP
