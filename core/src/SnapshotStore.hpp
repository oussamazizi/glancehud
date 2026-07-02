/// @file SnapshotStore.hpp
/// @brief Single-writer / multi-reader store that publishes snapshots.
#ifndef GLANCEHUD_SRC_SNAPSHOT_STORE_HPP
#define GLANCEHUD_SRC_SNAPSHOT_STORE_HPP

#include <atomic>
#include <memory>

#include "glancehud/Snapshot.hpp"

// std::atomic<std::shared_ptr<T>> requires a recent standard library
// (libstdc++ >= 12, libc++ >= 14). When available we use the truly lock-free
// path; otherwise we fall back to a short mutex. The public interface is
// identical either way, so callers never need to care.
#if defined(__cpp_lib_atomic_shared_ptr)
#define GLANCEHUD_HAS_ATOMIC_SHARED_PTR 1
#else
#define GLANCEHUD_HAS_ATOMIC_SHARED_PTR 0
#include <mutex>
#endif

namespace glance {

/// @brief Holds the latest @ref Snapshot for lock-free reads by renderers.
///
/// The sampler thread is the sole writer (@ref publish); any number of threads
/// may @ref load concurrently. Each publish swaps in a fresh immutable snapshot,
/// so readers never observe a torn value.
class SnapshotStore {
public:
    SnapshotStore();

    SnapshotStore(const SnapshotStore&) = delete;
    SnapshotStore& operator=(const SnapshotStore&) = delete;

    /// @brief Publish a new snapshot (writer thread).
    void publish(const Snapshot& snapshot);

    /// @brief Read the most recently published snapshot (any thread).
    [[nodiscard]] Snapshot load() const;

private:
#if GLANCEHUD_HAS_ATOMIC_SHARED_PTR
    std::atomic<std::shared_ptr<const Snapshot>> current_;
#else
    mutable std::mutex mutex_;
    std::shared_ptr<const Snapshot> current_;
#endif
};

}  // namespace glance

#endif  // GLANCEHUD_SRC_SNAPSHOT_STORE_HPP
