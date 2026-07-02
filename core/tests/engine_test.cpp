/// @file engine_test.cpp
/// @brief Behavioral tests for the core engine (store, FPS, sampling loop).
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <memory>
#include <thread>

#include "FpsCalculator.hpp"
#include "MetricsEngine.hpp"
#include "SnapshotStore.hpp"
#include "glancehud/IClock.hpp"
#include "glancehud/IMetricProvider.hpp"
#include "glancehud/IOverlayRenderer.hpp"

using namespace glance;
using namespace std::chrono_literals;

namespace {

/// Deterministic, manually-advanced clock.
class FakeClock : public IClock {
public:
    std::atomic<MetricSample::TimePoint> t{MetricSample::TimePoint{}};
    MetricSample::TimePoint now() const noexcept override { return t.load(); }
    void advance(std::chrono::nanoseconds d) { t.store(t.load() + d); }
};

/// Provider that always reports a fixed CPU value.
class ConstProvider : public IMetricProvider {
public:
    MetricType type() const noexcept override { return MetricType::CpuUsage; }
    MetricSample sample(MetricSample::TimePoint now) override {
        return {MetricType::CpuUsage, 42.0, now, true};
    }
};

/// Renderer that records how it was driven.
class CountingRenderer : public IOverlayRenderer {
public:
    std::atomic<int> renders{0};
    std::atomic<bool> visible{false};
    void render(const Snapshot&) override { renders.fetch_add(1); }
    void setVisible(bool v) override { visible.store(v); }
};

}  // namespace

TEST(SnapshotStore, StartsEmptyThenPublishes) {
    SnapshotStore store;
    EXPECT_EQ(store.load().sequence, 0u);

    Snapshot s;
    s.sequence = 7;
    s[MetricType::RamUsage] = {MetricType::RamUsage, 215.0, {}, true};
    store.publish(s);

    const auto got = store.load();
    EXPECT_EQ(got.sequence, 7u);
    EXPECT_DOUBLE_EQ(got[MetricType::RamUsage].value, 215.0);
}

TEST(FpsCalculator, ComputesSixtyFps) {
    FpsCalculator fps;
    EXPECT_FALSE(fps.hasData());

    auto t = MetricSample::TimePoint{};
    for (int i = 0; i < 120; ++i) {
        fps.onFrame(t);
        t += std::chrono::nanoseconds(16'666'667);  // 60 Hz
    }

    EXPECT_TRUE(fps.hasData());
    EXPECT_NEAR(fps.fps(), 60.0, 1.0);
    EXPECT_NEAR(fps.frameTimeMs(), 16.667, 0.1);
}

TEST(FpsCalculator, IgnoresNonPositiveDelta) {
    FpsCalculator fps;
    const auto t = MetricSample::TimePoint{};
    fps.onFrame(t);
    fps.onFrame(t);  // Same timestamp -> delta 0, ignored.
    EXPECT_FALSE(fps.hasData());
}

TEST(MetricsEngine, SamplesProvidersAndDrivesRenderer) {
    FakeClock clock;
    MetricsEngine engine(clock);

    Config cfg;
    cfg.sampleInterval = 5ms;
    cfg.autoShow = true;
    engine.setConfig(cfg);
    engine.addProvider(std::make_unique<ConstProvider>());

    auto renderer = std::make_unique<CountingRenderer>();
    auto* rendererRaw = renderer.get();
    engine.setRenderer(std::move(renderer));

    engine.start();
    EXPECT_TRUE(engine.isRunning());
    EXPECT_TRUE(rendererRaw->visible.load());  // autoShow

    Snapshot snap;
    for (int i = 0; i < 200 && !snap[MetricType::Fps].valid; ++i) {
        engine.onFrame();
        clock.advance(std::chrono::nanoseconds(16'666'667));
        std::this_thread::sleep_for(2ms);
        snap = engine.latest();
    }
    engine.stop();
    EXPECT_FALSE(engine.isRunning());

    EXPECT_GT(snap.sequence, 0u);
    EXPECT_TRUE(snap[MetricType::CpuUsage].valid);
    EXPECT_DOUBLE_EQ(snap[MetricType::CpuUsage].value, 42.0);
    EXPECT_TRUE(snap[MetricType::Fps].valid);
    EXPECT_GT(rendererRaw->renders.load(), 0);
}

TEST(MetricsEngine, StartStopIsIdempotent) {
    FakeClock clock;
    MetricsEngine engine(clock);
    engine.start();
    engine.start();  // no-op
    EXPECT_TRUE(engine.isRunning());
    engine.stop();
    engine.stop();  // no-op
    EXPECT_FALSE(engine.isRunning());
}
