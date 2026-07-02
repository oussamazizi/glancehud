<div align="center">

# GlanceHUD

**A lightweight, modern, cross-platform native performance overlay.**

FPS · Frame time · CPU · RAM · GPU · Network — live, on top of your app, during development.

[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)
![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C.svg)

</div>

---

> ⚠️ **Work in progress — early development.**
> The C++ **core** and the **Android** integration are functional. **iOS, Flutter
> and React Native are not implemented yet**, and GPU time can't be auto-collected
> yet (see [Project status](#project-status) for why).
> The example overlay below shows the intended full output; see
> [Project status](#project-status) for exactly what works today.

## Why GlanceHUD?

A light, modern alternative to heavyweight profilers. A tiny native C++20 engine
collects metrics with **no per-frame allocation** and publishes them lock-free to
a draggable, collapsible, resizable overlay that stays visible across your whole app.

```
+------------------------------+
| GlanceHUD                    |
| FPS : 120                    |
| CPU : 18%                    |
| RAM : 215 MB                 |
| GPU : 7 ms                   |
| NET : ↑120 ↓560 KB/s         |
+------------------------------+
```

## Platforms

| Platform     | Status              | Integration        |
|--------------|---------------------|--------------------|
| Android      | ✅ Functional (V1)  | JNI + Kotlin       |
| iOS          | ⬜ Not started      | Obj-C++ + Swift    |
| Flutter      | ⬜ Not started      | Plugin (dart:ffi)  |
| React Native | ⬜ Not started      | Bridge (JSI)       |

## Project status

Honest snapshot of what exists today — this is an early-stage project.

**Working**
- ✅ **Core (C++20)**: sampling engine, lock-free snapshot store, EMA-smoothed FPS,
  public C++ API + stable C ABI. Unit-tested (GoogleTest).
- ✅ **Android**: JNI bridge; CPU / RAM / network providers reading `/proc`;
  FPS via `Choreographer`; draggable, collapsible native overlay with
  performance color-coding; optional foreground service to persist the overlay;
  runnable example app; multi-ABI AAR build.

**Not done yet**
- ⬜ **iOS / Flutter / React Native** integrations (the core is ready for them).
- ⬜ **GPU time**: unlike CPU / RAM / network, it cannot be read from outside the
  app — there is no `/proc` counter for it, and GPU timing lives *inside* the
  render pipeline (OpenGL/Vulkan timer queries, which only the rendering app can
  issue). It will be supported through an opt-in API where the app reports its own
  GPU timings (like it already feeds frame ticks for FPS) — planned for V2.
- ⬜ **V2** analytics: FPS history & graph, JSON/CSV export, session recording,
  jank / FPS-drop / memory-leak detection, custom events.

Contributions and feedback are very welcome while this takes shape.

## Architecture at a glance

```
core (pure C++20, zero platform deps)   ──►   C ABI (Api.h)   ──►   JNI / ObjC++ / FFI / JSI
   MetricsEngine · Snapshot store              stable boundary        platform wrappers + native overlay
```

- **`core/`** owns no platform code. Platform metric sources implement
  [`IMetricProvider`](core/include/glancehud/IMetricProvider.hpp) and are injected
  (dependency inversion).
- A single **sampler thread** publishes an immutable [`Snapshot`](core/include/glancehud/Snapshot.hpp)
  atomically; renderers read it without locking.
- The FFI surface is a stable **C ABI** ([`Api.h`](core/include/glancehud/Api.h)) so
  bindings evolve without recompiling the engine.

See [docs/architecture.md](docs/architecture.md) for the full design.

## Public API (C++)

```cpp
#include <glancehud/PerfOverlay.hpp>

auto& overlay = glance::PerfOverlay::instance();
overlay.initialize({});   // defaults
overlay.start();
overlay.show();
// ... per frame, from the host frame callback:
overlay.onFrame();
// ...
overlay.setOpacity(0.7f);
overlay.setPosition(24.0f, 48.0f);
overlay.stop();
```

## Build (core)

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset ci
```

Requirements: CMake ≥ 3.22, a C++20 compiler, Ninja.

## Roadmap

**Done** — C++ core (FPS, frame time, CPU, RAM, network) + Android integration
with a draggable / collapsible native overlay and optional foreground service.

**Next** — iOS integration (`mach` providers + `UIWindow` overlay), then Flutter
and React Native wrappers over the C ABI; GPU time where available.

**V2** — FPS history & real-time graph, JSON/CSV export, session recording, automatic
FPS-drop / jank / memory-leak detection, network analysis, custom events. The
`history/`, `export/` and `detect/` folders already exist so these land without a refactor.

## Contributing

Contributions are welcome — see [CONTRIBUTING.md](CONTRIBUTING.md) and the
[Code of Conduct](CODE_OF_CONDUCT.md).

## License

Apache-2.0 — see [LICENSE](LICENSE).
