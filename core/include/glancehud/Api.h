/**
 * @file Api.h
 * @brief Stable C ABI for GlanceHUD.
 *
 * This is the single, versioned boundary consumed by all bindings:
 *   - Android JNI, iOS Objective-C++
 *   - Flutter (dart:ffi), React Native (JSI/TurboModules)
 *
 * Keeping the FFI surface in plain C avoids C++ ABI fragility and lets wrappers
 * evolve without recompiling the engine. All functions forward to
 * glance::PerfOverlay::instance().
 */
#ifndef GLANCEHUD_API_H
#define GLANCEHUD_API_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Anchor corners, mirrors glance::Anchor. */
typedef enum {
    GLANCE_ANCHOR_TOP_LEFT = 0,
    GLANCE_ANCHOR_TOP_RIGHT = 1,
    GLANCE_ANCHOR_BOTTOM_LEFT = 2,
    GLANCE_ANCHOR_BOTTOM_RIGHT = 3
} GlanceAnchor;

/** @brief Plain-C configuration, mirrors glance::Config. */
typedef struct {
    int sample_interval_ms; /**< Sampling period for CPU/RAM/net. */
    float opacity;          /**< Overlay opacity in [0, 1]. */
    GlanceAnchor anchor;    /**< Initial anchor corner. */
    int start_collapsed;    /**< Boolean: start collapsed. */
    int auto_show;          /**< Boolean: show after start. */
} GlanceConfig;

/** @brief One metric value plus whether it is currently available. */
typedef struct {
    double value; /**< Value in the metric's canonical unit. */
    int valid;    /**< Boolean: 0 when the metric is unavailable. */
} GlanceMetric;

/** @brief Flat readout of the latest snapshot, for FFI consumers. */
typedef struct {
    unsigned long long sequence; /**< Publish counter (drop detection). */
    GlanceMetric fps;
    GlanceMetric frame_time;
    GlanceMetric cpu;
    GlanceMetric ram;
    GlanceMetric gpu;
    GlanceMetric net_up;
    GlanceMetric net_down;
} GlanceSnapshot;

/** @brief Semantic version string ("0.1.0"). Valid for the process lifetime. */
const char* glance_version(void);

/** @brief Configure the engine. Pass NULL for defaults. */
void glance_initialize(const GlanceConfig* config);

/** @brief Start / stop the sampler thread. */
void glance_start(void);
void glance_stop(void);

/** @brief Show / hide the overlay. */
void glance_show(void);
void glance_hide(void);

/** @brief Overlay placement and appearance. */
void glance_set_position(float x, float y);
void glance_set_opacity(float opacity);

/** @brief Notify a host frame (drives FPS / frame time / jank). */
void glance_on_frame(void);

/** @brief Copy the latest metric values into @p out (must be non-NULL). */
void glance_snapshot(GlanceSnapshot* out);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* GLANCEHUD_API_H */
