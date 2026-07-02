# GlanceHUD — Android

Native Android integration: a JNI bridge to the shared C++ core, the
Android-specific metric providers (CPU/RAM via `/proc`), and a floating overlay
window drawn natively.

## How it fits together

```
Kotlin API (GlanceHud)                     ── the app calls this
   │  external funcs
   ▼
JNI bridge (glance_jni.cpp) ──► C-ABI / core (glance::PerfOverlay)
   ▲                                   ▲
Choreographer (one tick per frame)     │ injected at init
   feeds onFrame() → FPS         CpuProvider / RamProvider  (read /proc)

Rendering: Kotlin polls nativeSnapshot() every frame and draws
GlanceOverlayView in a WindowManager overlay (draggable, tap to collapse).
```

The core reads no hardware: the Android providers are its "senses", injected in
`nativeInitialize`. FPS is computed in the core from the Choreographer tick.

## Usage

```kotlin
import com.glancehud.GlanceHud
import com.glancehud.GlanceConfig

// Once, e.g. in Application.onCreate() or your debug Activity:
GlanceHud.initialize(
    context,
    GlanceConfig(sampleIntervalMs = 500, opacity = 0.85f, autoShow = true),
)

// The overlay floats over other apps, which needs a runtime permission:
if (!GlanceHud.hasOverlayPermission(context)) {
    GlanceHud.requestOverlayPermission(activity)  // opens the system settings screen
}

GlanceHud.start()   // begins sampling + shows the overlay
// ...
GlanceHud.setOpacity(0.7f)
GlanceHud.setPosition(24, 48)
// ...
GlanceHud.stop()    // stops sampling + removes the overlay
```

> ⚠️ Ship GlanceHUD in **debug builds only**. It requests `SYSTEM_ALERT_WINDOW`
> and is a development tool.

## Build

Requires Android Studio (or the Android SDK + NDK + CMake). From `android/`:

```bash
./gradlew assembleRelease   # produces the .aar under build/outputs/aar/
```

The native build (`src/main/cpp/CMakeLists.txt`) reuses the exact same
`../../../../core` sources compiled by the desktop/CI build — one engine, many
targets. `abiFilters` builds `arm64-v8a`, `armeabi-v7a`, `x86_64`; each device
installs only the matching one.

## What's implemented

- ✅ JNI bridge, CPU & RAM providers, FPS via Choreographer
- ✅ Draggable / collapsible native overlay
- ⬜ GPU time and network providers (planned)
- ⬜ Persistent overlay across process/activity changes (foreground service)
