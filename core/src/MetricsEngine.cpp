#include "MetricsEngine.hpp"

#include <utility>

namespace glance {

MetricsEngine::MetricsEngine(IClock& clock) : clock_(clock) {}

MetricsEngine::~MetricsEngine() {
    stop();
}

void MetricsEngine::setConfig(const Config& config) {
    config_ = config;
}

void MetricsEngine::addProvider(std::unique_ptr<IMetricProvider> provider) {
    if (provider) {
        providers_.push_back(std::move(provider));
    }
}

void MetricsEngine::setRenderer(std::unique_ptr<IOverlayRenderer> renderer) {
    std::lock_guard<std::mutex> lock(mutex_);
    renderer_ = std::move(renderer);
}

void MetricsEngine::start() {
    if (running_.exchange(true, std::memory_order_acq_rel)) {
        return;  // Already running.
    }
    sequence_ = 0;
    fps_.reset();
    thread_ = std::thread(&MetricsEngine::run, this);

    if (config_.autoShow) {
        setVisible(true);
    }
}

void MetricsEngine::stop() {
    if (!running_.exchange(false, std::memory_order_acq_rel)) {
        return;  // Not running.
    }
    cv_.notify_all();
    if (thread_.joinable()) {
        thread_.join();
    }
}

void MetricsEngine::setVisible(bool visible) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (renderer_) {
        renderer_->setVisible(visible);
    }
}

void MetricsEngine::onFrame() {
    fps_.onFrame(clock_.now());
}

Snapshot MetricsEngine::buildSnapshot(MetricSample::TimePoint now) {
    Snapshot snapshot;
    snapshot.sequence = ++sequence_;

    // Platform-provided metrics (CPU/RAM/GPU/network).
    for (const auto& provider : providers_) {
        snapshot[provider->type()] = provider->sample(now);
    }

    // Frame-derived metrics, computed in the core from onFrame() timestamps.
    if (fps_.hasData()) {
        snapshot[MetricType::Fps] = {MetricType::Fps, fps_.fps(), now, true};
        snapshot[MetricType::FrameTime] = {MetricType::FrameTime, fps_.frameTimeMs(), now, true};
    }

    return snapshot;
}

void MetricsEngine::run() {
    while (running_.load(std::memory_order_acquire)) {
        const auto now = clock_.now();
        const Snapshot snapshot = buildSnapshot(now);
        store_.publish(snapshot);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (renderer_) {
                renderer_->render(snapshot);
            }
        }

        // Interruptible sleep: wakes early when stop() flips running_.
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, config_.sampleInterval,
                     [this] { return !running_.load(std::memory_order_acquire); });
    }
}

}  // namespace glance
