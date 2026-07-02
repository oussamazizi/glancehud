/// @file Api.cpp
/// @brief C-ABI implementation: forwards every call to the process-wide facade.
#include "glancehud/Api.h"

#include <chrono>

#include "glancehud/Config.hpp"
#include "glancehud/PerfOverlay.hpp"
#include "glancehud/Version.hpp"

namespace {

glance::Anchor toAnchor(GlanceAnchor anchor) {
    switch (anchor) {
        case GLANCE_ANCHOR_TOP_LEFT:     return glance::Anchor::TopLeft;
        case GLANCE_ANCHOR_TOP_RIGHT:    return glance::Anchor::TopRight;
        case GLANCE_ANCHOR_BOTTOM_LEFT:  return glance::Anchor::BottomLeft;
        case GLANCE_ANCHOR_BOTTOM_RIGHT: return glance::Anchor::BottomRight;
    }
    return glance::Anchor::TopRight;
}

GlanceMetric toMetric(const glance::MetricSample& s) {
    return GlanceMetric{s.value, s.valid ? 1 : 0};
}

glance::Config toConfig(const GlanceConfig& c) {
    glance::Config config;
    config.sampleInterval = std::chrono::milliseconds(c.sample_interval_ms);
    config.opacity = c.opacity;
    config.anchor = toAnchor(c.anchor);
    config.startCollapsed = c.start_collapsed != 0;
    config.autoShow = c.auto_show != 0;
    return config;
}

}  // namespace

extern "C" {

const char* glance_version(void) {
    // The underlying string_view refers to a string literal, so .data() is a
    // valid, NUL-terminated C string for the lifetime of the process.
    return glance::version().data();
}

void glance_initialize(const GlanceConfig* config) {
    glance::PerfOverlay::instance().initialize(config != nullptr ? toConfig(*config)
                                                                 : glance::Config{});
}

void glance_start(void) { glance::PerfOverlay::instance().start(); }
void glance_stop(void) { glance::PerfOverlay::instance().stop(); }
void glance_show(void) { glance::PerfOverlay::instance().show(); }
void glance_hide(void) { glance::PerfOverlay::instance().hide(); }

void glance_set_position(float x, float y) {
    glance::PerfOverlay::instance().setPosition(x, y);
}

void glance_set_opacity(float opacity) {
    glance::PerfOverlay::instance().setOpacity(opacity);
}

void glance_on_frame(void) { glance::PerfOverlay::instance().onFrame(); }

void glance_snapshot(GlanceSnapshot* out) {
    if (out == nullptr) {
        return;
    }
    const glance::Snapshot snap = glance::PerfOverlay::instance().latest();
    out->sequence = snap.sequence;
    out->fps = toMetric(snap[glance::MetricType::Fps]);
    out->frame_time = toMetric(snap[glance::MetricType::FrameTime]);
    out->cpu = toMetric(snap[glance::MetricType::CpuUsage]);
    out->ram = toMetric(snap[glance::MetricType::RamUsage]);
    out->gpu = toMetric(snap[glance::MetricType::GpuTime]);
    out->net_up = toMetric(snap[glance::MetricType::NetUpload]);
    out->net_down = toMetric(snap[glance::MetricType::NetDownload]);
}

}  // extern "C"
