# GlanceHUD — Architecture

## Goals

- A tiny, fast, modern native performance overlay usable during development.
- One shared engine, many platforms (Android, iOS, Flutter, React Native).
- Zero technical debt: modular, testable, no unnecessary allocation, RAII everywhere.

## Layered design

```
┌──────────────────────────────────────────────────────────────┐
│                        core/  (pure C++20)                     │
│  PerfOverlay (facade)                                           │
│  MetricsEngine  ── sampler thread ──► SnapshotStore (lock-free) │
│  Contracts: IMetricProvider · IClock · IOverlayRenderer         │
│  V2 (folders reserved): history/ · export/ · detect/            │
└───────▲───────────────────────────────────────────▲──────────┘
        │ injects providers / renderer                │ reads Snapshot (vsync)
┌───────┴───────────┐                        ┌────────┴───────────┐
│ Platform providers │  (JNI / Obj-C++)       │  Native overlay    │
│ CPU · RAM · GPU ·  │                        │  renderer (V1)     │
│ Network            │                        │                    │
└───────▲───────────┘                        └────────▲───────────┘
        │                                              │
        └──── C ABI (Api.h) ── JNI / ObjC++ / dart:ffi / JSI ──────┘
```

### 1. Core is platform-free (Dependency Inversion)

The core defines interfaces and never links platform SDKs. OS-specific metrics
(CPU, RAM, GPU, network) are implemented in the platform modules as
`IMetricProvider`s and **injected** at startup via `PerfOverlay::addProvider`.
This keeps the engine unit-testable on any desktop CI and portable to future
platforms.

### 2. Threading model

A single **sampler thread** wakes on `Config::sampleInterval` and, per tick:

1. calls each registered provider's `sample()`,
2. assembles an immutable `Snapshot`,
3. publishes it atomically (`std::atomic<std::shared_ptr<const Snapshot>>` or a
   triple buffer).

Renderers read the latest snapshot **without locking** and with **no allocation
on the hot path**. Frame-based metrics (FPS, frame time, jank) are driven by
`PerfOverlay::onFrame()`, called from the host's frame callback
(Android `Choreographer`, iOS `CADisplayLink`, Flutter `SchedulerBinding`).

### 3. Rendering (V1 = native per platform)

The core produces a **view-model** (formatted labels + values). V1 draws it
natively:

- **Android**: an overlay `View` (Jetpack Compose / Canvas) hosted via
  `WindowManager`.
- **iOS**: a transparent `UIWindow` at a high `windowLevel`.

Dragging, collapsing, and resizing use each platform's gesture system for a
native feel. The `IOverlayRenderer` abstraction is retained so a **shared C++ GPU
renderer** (OpenGL ES / Metal / Vulkan) can be added in V2 without changing the
engine or the public API.

### 4. Stable C ABI as the FFI boundary

All bindings go through `Api.h` (`extern "C"`). Plain C avoids C++ ABI fragility
and lets Kotlin/Swift/Dart/JS wrappers evolve without recompiling the engine.

## V2 extensibility

The following land inside reserved folders without structural refactor:

| Feature                     | Location        |
|-----------------------------|-----------------|
| FPS history / graph         | `core/src/history/` (`RingBuffer`) |
| JSON / CSV export           | `core/src/export/` |
| Session recording           | `core/src/export/` |
| FPS-drop / jank detection   | `core/src/detect/` |
| Memory-leak heuristics      | `core/src/detect/` |
| Custom events               | engine event bus  |

## Design principles

- RAII and smart pointers throughout; no raw ownership.
- No allocation on the per-frame path.
- Interfaces over concretions at every platform seam.
- Everything the engine does is testable with a fake `IClock` and fake providers.
