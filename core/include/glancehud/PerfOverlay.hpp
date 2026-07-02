/// @file PerfOverlay.hpp
/// @brief Public C++ facade for GlanceHUD.
///
/// This is the primary entry point used by native (Android/iOS) integrations.
/// The FFI/bridge layers (Flutter, React Native) go through the C-ABI in
/// @ref Api.h, which forwards to this facade.
#ifndef GLANCEHUD_PERF_OVERLAY_HPP
#define GLANCEHUD_PERF_OVERLAY_HPP

#include <memory>

#include "glancehud/Config.hpp"
#include "glancehud/IMetricProvider.hpp"
#include "glancehud/IOverlayRenderer.hpp"
#include "glancehud/Snapshot.hpp"

namespace glance {

/// @brief High-level controller of the metrics engine and overlay lifecycle.
///
/// Thread-safe. A single instance is expected per process (see @ref instance),
/// but the class is not a hard singleton so it can be constructed in tests.
///
/// Lifecycle: @ref initialize once, then @ref start / @ref stop as needed.
/// @code
///   glance::PerfOverlay::instance().initialize({});
///   glance::PerfOverlay::instance().start();
/// @endcode
class PerfOverlay {
public:
    PerfOverlay();
    ~PerfOverlay();

    PerfOverlay(const PerfOverlay&) = delete;
    PerfOverlay& operator=(const PerfOverlay&) = delete;
    PerfOverlay(PerfOverlay&&) = delete;
    PerfOverlay& operator=(PerfOverlay&&) = delete;

    /// @brief Process-wide instance used by the C-ABI and platform wrappers.
    [[nodiscard]] static PerfOverlay& instance();

    /// @brief Configure the engine. Safe to call once before @ref start.
    void initialize(const Config& config);

    /// @brief Register a platform metric source (takes ownership).
    /// @note Call before @ref start. Providers are the DIP boundary that keeps
    ///       the core free of platform code.
    void addProvider(std::unique_ptr<IMetricProvider> provider);

    /// @brief Set the renderer that draws the overlay (takes ownership).
    void setRenderer(std::unique_ptr<IOverlayRenderer> renderer);

    /// @brief Begin sampling on the background thread.
    void start();

    /// @brief Stop sampling and join the background thread.
    void stop();

    /// @brief Show the overlay.
    void show();

    /// @brief Hide the overlay without stopping sampling.
    void hide();

    /// @brief Move the overlay to screen coordinates (device-independent px).
    void setPosition(float x, float y);

    /// @brief Set overlay opacity in [0, 1].
    void setOpacity(float opacity);

    /// @brief Feed a frame timestamp from the host frame callback
    ///        (Android Choreographer / iOS CADisplayLink / Flutter scheduler).
    ///        Drives FPS, frame time and jank computation.
    void onFrame();

    /// @brief Latest published snapshot (lock-free read).
    [[nodiscard]] Snapshot latest() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace glance

#endif  // GLANCEHUD_PERF_OVERLAY_HPP
