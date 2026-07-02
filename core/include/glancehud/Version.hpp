/// @file Version.hpp
/// @brief Compile-time and runtime version information.
#ifndef GLANCEHUD_VERSION_HPP
#define GLANCEHUD_VERSION_HPP

#include <string_view>

namespace glance {

inline constexpr int kVersionMajor = 0;
inline constexpr int kVersionMinor = 1;
inline constexpr int kVersionPatch = 0;

/// @brief Semantic version string, e.g. "0.1.0".
[[nodiscard]] std::string_view version() noexcept;

}  // namespace glance

#endif  // GLANCEHUD_VERSION_HPP
