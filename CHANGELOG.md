# Changelog

All notable changes to GlanceHUD are documented here.
Format based on [Keep a Changelog](https://keepachangelog.com/), versioning follows [SemVer](https://semver.org/).

## [Unreleased]

### Added
- Initial repository scaffold: monorepo structure, CMake build, CI, docs.
- Core public API contracts: `MetricType`, `MetricSample`, `Snapshot`,
  `IMetricProvider`, `IClock`, `IOverlayRenderer`, `Config`, `PerfOverlay`,
  C-ABI `Api.h`.
- Core engine implementation:
  - `SnapshotStore` — single-writer/multi-reader publish (lock-free when the
    standard library provides `std::atomic<std::shared_ptr>`, mutex fallback
    otherwise).
  - `FpsCalculator` — EMA-smoothed FPS and frame time from frame timestamps.
  - `MetricsEngine` — RAII sampler thread with interruptible sleep, provider
    sampling, snapshot publishing and renderer driving.
  - `PerfOverlay` facade (pimpl) + `SteadyClock` + C-ABI implementation.
- Unit tests (GoogleTest): store, FPS calculator, engine end-to-end — 8 passing.
- C-ABI snapshot readout (`GlanceSnapshot`, `glance_snapshot`) for FFI consumers.
- Android module:
  - JNI bridge (`glance_jni.cpp`) mapping Kotlin ↔ core, injecting providers.
  - `CpuProvider` / `RamProvider` / `NetProvider` (upload+download via a shared
    `/proc/net/dev` reader) — all verified against real data.
  - Kotlin API (`GlanceHud`, `GlanceConfig`) + native floating overlay
    (`GlanceOverlayManager`, `GlanceOverlayView`): draggable, tap-to-collapse,
    per-frame updates via `Choreographer`.
  - Gradle library build producing an AAR (multi-ABI), CMake reusing core.
  - Overlay color-codes metrics by performance (FPS/CPU green/orange/red).
  - `GlanceOverlayService` foreground service (`startPersistent`) keeps the
    overlay alive when the host app is backgrounded or killed.
  - Runnable example app (`android/example/`) with an APK buildable via
    `./gradlew :example:assembleDebug`.
