/// @file IOverlayRenderer.hpp
/// @brief Contract for drawing the overlay; native in V1, GPU-shared in V2.
#ifndef GLANCEHUD_IOVERLAY_RENDERER_HPP
#define GLANCEHUD_IOVERLAY_RENDERER_HPP

#include "glancehud/Snapshot.hpp"

namespace glance {

/// @brief Renders a @ref Snapshot to the screen.
///
/// V1 uses native per-platform implementations (Android View, iOS UIWindow)
/// that read the shared view-model. Keeping this interface lets a shared C++
/// GPU renderer (OpenGL ES / Metal / Vulkan) be added in V2 without touching
/// the engine or the public API.
class IOverlayRenderer {
public:
    virtual ~IOverlayRenderer() = default;

    /// @brief Push the latest snapshot to be displayed. Must be non-blocking.
    virtual void render(const Snapshot& snapshot) = 0;

    /// @brief Toggle overlay visibility.
    virtual void setVisible(bool visible) = 0;
};

}  // namespace glance

#endif  // GLANCEHUD_IOVERLAY_RENDERER_HPP
