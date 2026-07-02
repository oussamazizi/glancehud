# Getting Started (core)

> Platform integrations (Android/iOS/Flutter/RN) will be documented as they land.
> For now you can build and test the pure C++ core.

## Prerequisites

- CMake ≥ 3.22
- A C++20 compiler (Clang 14+, GCC 11+, MSVC 19.3+)
- Ninja

## Build & test

```bash
git clone https://github.com/oussamazizi/glancehud.git
cd glancehud

cmake --preset debug
cmake --build --preset debug
ctest --preset ci
```

## Minimal usage

```cpp
#include <glancehud/PerfOverlay.hpp>

int main() {
    auto& overlay = glance::PerfOverlay::instance();
    overlay.initialize({});   // default Config
    overlay.start();
    overlay.show();

    // In your render loop, once per frame:
    // overlay.onFrame();

    overlay.stop();
}
```

On a real device, platform providers (CPU/RAM/GPU/network) and the native overlay
renderer are wired up by the Android/iOS wrapper — you only call the lifecycle API.
