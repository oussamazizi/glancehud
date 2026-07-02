#include "glancehud/PerfOverlay.hpp"

#include <algorithm>
#include <utility>

#include "MetricsEngine.hpp"
#include "SteadyClock.hpp"

namespace glance {

/// @brief Private implementation (pimpl) hidden from the public header.
///
/// Keeping the engine, clock and appearance state here means the public
/// @ref PerfOverlay header stays free of internal includes, so changing the
/// implementation never forces callers to recompile.
struct PerfOverlay::Impl {
    SteadyClock clock;
    MetricsEngine engine{clock};

    // Appearance state. The native renderer reads these when wiring the
    // platform overlay; the core simply tracks them.
    float opacity{0.85f};
    float positionX{0.0f};
    float positionY{0.0f};
    bool positionSet{false};
};

PerfOverlay::PerfOverlay() : impl_(std::make_unique<Impl>()) {}
PerfOverlay::~PerfOverlay() = default;

PerfOverlay& PerfOverlay::instance() {
    static PerfOverlay instance;
    return instance;
}

void PerfOverlay::initialize(const Config& config) {
    impl_->opacity = std::clamp(config.opacity, 0.0F, 1.0F);
    impl_->engine.setConfig(config);
}

void PerfOverlay::addProvider(std::unique_ptr<IMetricProvider> provider) {
    impl_->engine.addProvider(std::move(provider));
}

void PerfOverlay::setRenderer(std::unique_ptr<IOverlayRenderer> renderer) {
    impl_->engine.setRenderer(std::move(renderer));
}

void PerfOverlay::start() {
    impl_->engine.start();
}

void PerfOverlay::stop() {
    impl_->engine.stop();
}

void PerfOverlay::show() {
    impl_->engine.setVisible(true);
}

void PerfOverlay::hide() {
    impl_->engine.setVisible(false);
}

void PerfOverlay::setPosition(float x, float y) {
    impl_->positionX = x;
    impl_->positionY = y;
    impl_->positionSet = true;
}

void PerfOverlay::setOpacity(float opacity) {
    impl_->opacity = std::clamp(opacity, 0.0F, 1.0F);
}

void PerfOverlay::onFrame() {
    impl_->engine.onFrame();
}

Snapshot PerfOverlay::latest() const {
    return impl_->engine.latest();
}

}  // namespace glance
