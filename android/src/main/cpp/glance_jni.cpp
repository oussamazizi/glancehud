/// @file glance_jni.cpp
/// @brief JNI bridge: maps Kotlin `external` calls onto the C++ core, and wires
///        the Android-specific metric providers into the engine.
///
/// The rendering is intentionally NOT bridged from C++ to Java: Kotlin polls the
/// latest snapshot every frame (see nativeSnapshot) and draws natively. This
/// avoids fragile C++→JVM callbacks and keeps the core UI-agnostic.
#include <jni.h>

#include <chrono>
#include <cmath>
#include <memory>

#include "glancehud/Config.hpp"
#include "glancehud/PerfOverlay.hpp"

#include "providers/CpuProvider.hpp"
#include "providers/NetProvider.hpp"
#include "providers/RamProvider.hpp"

namespace {

/// Encode an unavailable metric as NaN so Kotlin can test with isNaN().
jdouble encode(const glance::MetricSample& s) {
    return s.valid ? s.value : std::nan("");
}

bool g_providersRegistered = false;

}  // namespace

extern "C" {

JNIEXPORT void JNICALL Java_com_glancehud_GlanceHud_nativeInitialize(
    JNIEnv* /*env*/, jobject /*thiz*/, jint intervalMs, jfloat opacity,
    jboolean autoShow, jboolean startCollapsed) {
    auto& overlay = glance::PerfOverlay::instance();

    // Register the platform providers exactly once. This is where the pure core
    // is fed its Android-specific "senses".
    if (!g_providersRegistered) {
        overlay.addProvider(std::make_unique<glance::android::CpuProvider>());
        overlay.addProvider(std::make_unique<glance::android::RamProvider>());

        // Upload/download share one reader of /proc/net/dev.
        auto netSampler = std::make_shared<glance::android::NetSampler>();
        overlay.addProvider(std::make_unique<glance::android::NetUploadProvider>(netSampler));
        overlay.addProvider(std::make_unique<glance::android::NetDownloadProvider>(netSampler));

        g_providersRegistered = true;
    }

    glance::Config config;
    config.sampleInterval = std::chrono::milliseconds(intervalMs);
    config.opacity = opacity;
    config.autoShow = autoShow == JNI_TRUE;
    config.startCollapsed = startCollapsed == JNI_TRUE;
    overlay.initialize(config);
}

JNIEXPORT void JNICALL Java_com_glancehud_GlanceHud_nativeStart(JNIEnv*, jobject) {
    glance::PerfOverlay::instance().start();
}

JNIEXPORT void JNICALL Java_com_glancehud_GlanceHud_nativeStop(JNIEnv*, jobject) {
    glance::PerfOverlay::instance().stop();
}

JNIEXPORT void JNICALL Java_com_glancehud_GlanceHud_nativeOnFrame(JNIEnv*, jobject) {
    glance::PerfOverlay::instance().onFrame();
}

/// Returns { sequence, fps, frameTime, cpu, ram, gpu, netUp, netDown }.
/// Unavailable metrics are NaN.
JNIEXPORT jdoubleArray JNICALL
Java_com_glancehud_GlanceHud_nativeSnapshot(JNIEnv* env, jobject) {
    const glance::Snapshot snap = glance::PerfOverlay::instance().latest();

    jdouble buffer[8];
    buffer[0] = static_cast<jdouble>(snap.sequence);
    buffer[1] = encode(snap[glance::MetricType::Fps]);
    buffer[2] = encode(snap[glance::MetricType::FrameTime]);
    buffer[3] = encode(snap[glance::MetricType::CpuUsage]);
    buffer[4] = encode(snap[glance::MetricType::RamUsage]);
    buffer[5] = encode(snap[glance::MetricType::GpuTime]);
    buffer[6] = encode(snap[glance::MetricType::NetUpload]);
    buffer[7] = encode(snap[glance::MetricType::NetDownload]);

    jdoubleArray result = env->NewDoubleArray(8);
    if (result != nullptr) {
        env->SetDoubleArrayRegion(result, 0, 8, buffer);
    }
    return result;
}

}  // extern "C"
