#include "SnapshotStore.hpp"

namespace glance {

SnapshotStore::SnapshotStore() : current_(std::make_shared<const Snapshot>()) {}

void SnapshotStore::publish(const Snapshot& snapshot) {
    auto fresh = std::make_shared<const Snapshot>(snapshot);
#if GLANCEHUD_HAS_ATOMIC_SHARED_PTR
    current_.store(std::move(fresh), std::memory_order_release);
#else
    std::lock_guard<std::mutex> lock(mutex_);
    current_ = std::move(fresh);
#endif
}

Snapshot SnapshotStore::load() const {
#if GLANCEHUD_HAS_ATOMIC_SHARED_PTR
    return *current_.load(std::memory_order_acquire);
#else
    std::lock_guard<std::mutex> lock(mutex_);
    return *current_;
#endif
}

}  // namespace glance
