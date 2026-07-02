/// @file MetricsEngine.hpp
/// @brief The sampling engine: owns providers, the sampler thread and the store.
#ifndef GLANCEHUD_SRC_METRICS_ENGINE_HPP
#define GLANCEHUD_SRC_METRICS_ENGINE_HPP

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "glancehud/Config.hpp"
#include "glancehud/IClock.hpp"
#include "glancehud/IMetricProvider.hpp"
#include "glancehud/IOverlayRenderer.hpp"
#include "glancehud/Snapshot.hpp"

#include "FpsCalculator.hpp"
#include "SnapshotStore.hpp"

namespace glance {

/// @brief Drives metric collection on a background thread.
///
/// Ownership model (RAII): the engine owns its providers, renderer and thread.
/// The destructor stops the thread, so no leak or dangling thread is possible.
///
/// Threading contract:
///  - Register providers/renderer and call @ref setConfig **before** @ref start.
///  - @ref onFrame is called from the UI thread every frame.
///  - @ref latest and @ref setVisible are safe from any thread.
class MetricsEngine {
public:
    explicit MetricsEngine(IClock& clock);
    ~MetricsEngine();

    MetricsEngine(const MetricsEngine&) = delete;
    MetricsEngine& operator=(const MetricsEngine&) = delete;
    MetricsEngine(MetricsEngine&&) = delete;
    MetricsEngine& operator=(MetricsEngine&&) = delete;

    /// @brief Apply configuration. Call before @ref start.
    void setConfig(const Config& config);

    /// @brief Register a metric source (before @ref start). Takes ownership.
    void addProvider(std::unique_ptr<IMetricProvider> provider);

    /// @brief Set the overlay renderer. Takes ownership.
    void setRenderer(std::unique_ptr<IOverlayRenderer> renderer);

    /// @brief Launch the sampler thread. No-op if already running.
    void start();

    /// @brief Stop and join the sampler thread. No-op if not running.
    void stop();

    /// @brief Whether the sampler thread is active.
    [[nodiscard]] bool isRunning() const noexcept {
        return running_.load(std::memory_order_acquire);
    }

    /// @brief Toggle overlay visibility (any thread).
    void setVisible(bool visible);

    /// @brief Notify a host frame (drives FPS / frame time).
    void onFrame();

    /// @brief Latest published snapshot (lock-free read).
    [[nodiscard]] Snapshot latest() const { return store_.load(); }

private:
    void run();  ///< Sampler loop body.
    [[nodiscard]] Snapshot buildSnapshot(MetricSample::TimePoint now);

    IClock& clock_;
    Config config_{};
    std::vector<std::unique_ptr<IMetricProvider>> providers_;
    std::unique_ptr<IOverlayRenderer> renderer_;

    SnapshotStore store_;
    FpsCalculator fps_;
    std::uint64_t sequence_{0};  ///< Written only by the sampler thread.

    std::thread thread_;
    mutable std::mutex mutex_;  ///< Guards renderer_ access and the sleep wait.
    std::condition_variable cv_;
    std::atomic<bool> running_{false};
};

}  // namespace glance

#endif  // GLANCEHUD_SRC_METRICS_ENGINE_HPP
