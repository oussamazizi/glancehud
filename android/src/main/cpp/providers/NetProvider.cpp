#include "providers/NetProvider.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <chrono>

namespace glance::android {
namespace {

/// Sum received/transmitted bytes across all interfaces except loopback,
/// from /proc/net/dev. Returns false if the file cannot be read.
bool readTotals(std::uint64_t& rxOut, std::uint64_t& txOut) {
    std::FILE* file = std::fopen("/proc/net/dev", "re");
    if (file == nullptr) {
        return false;
    }

    char line[512];
    // Skip the two header lines.
    if (std::fgets(line, sizeof(line), file) == nullptr ||
        std::fgets(line, sizeof(line), file) == nullptr) {
        std::fclose(file);
        return false;
    }

    std::uint64_t rxTotal = 0;
    std::uint64_t txTotal = 0;
    while (std::fgets(line, sizeof(line), file) != nullptr) {
        char* colon = std::strchr(line, ':');
        if (colon == nullptr) {
            continue;
        }
        *colon = '\0';

        // Interface name is before the colon (may be right-padded with spaces).
        char* name = line;
        while (*name == ' ') {
            ++name;
        }
        if (std::strcmp(name, "lo") == 0) {
            continue;  // Ignore loopback.
        }

        // Receive columns (8): bytes packets errs drop fifo frame compressed multicast
        // Transmit columns (8): bytes packets errs drop fifo colls carrier compressed
        unsigned long long fields[16] = {0};
        const int matched = std::sscanf(
            colon + 1,
            "%llu %llu %llu %llu %llu %llu %llu %llu"
            "%llu %llu %llu %llu %llu %llu %llu %llu",
            &fields[0], &fields[1], &fields[2], &fields[3], &fields[4], &fields[5],
            &fields[6], &fields[7], &fields[8], &fields[9], &fields[10], &fields[11],
            &fields[12], &fields[13], &fields[14], &fields[15]);
        if (matched >= 9) {
            rxTotal += static_cast<std::uint64_t>(fields[0]);   // receive bytes
            txTotal += static_cast<std::uint64_t>(fields[8]);   // transmit bytes
        }
    }
    std::fclose(file);

    rxOut = rxTotal;
    txOut = txTotal;
    return true;
}

}  // namespace

void NetSampler::update(MetricSample::TimePoint now) {
    // Only refresh once per tick, even though two providers call us.
    if (updatedForValid_ && updatedFor_ == now) {
        return;
    }
    updatedFor_ = now;
    updatedForValid_ = true;

    std::uint64_t rx = 0;
    std::uint64_t tx = 0;
    if (!readTotals(rx, tx)) {
        valid_ = false;
        return;
    }

    if (!haveBaseline_) {
        lastRx_ = rx;
        lastTx_ = tx;
        lastNow_ = now;
        haveBaseline_ = true;
        valid_ = false;  // Need two samples for a rate.
        return;
    }

    const std::chrono::duration<double> seconds = now - lastNow_;
    const double dt = seconds.count();
    lastNow_ = now;

    const std::uint64_t deltaRx = rx - lastRx_;
    const std::uint64_t deltaTx = tx - lastTx_;
    lastRx_ = rx;
    lastTx_ = tx;

    if (dt <= 0.0) {
        valid_ = false;
        return;
    }

    downloadKBs_ = static_cast<double>(deltaRx) / 1024.0 / dt;
    uploadKBs_ = static_cast<double>(deltaTx) / 1024.0 / dt;
    valid_ = true;
}

MetricSample NetUploadProvider::sample(MetricSample::TimePoint now) {
    sampler_->update(now);
    return {MetricType::NetUpload, sampler_->uploadKBs(), now, sampler_->valid()};
}

MetricSample NetDownloadProvider::sample(MetricSample::TimePoint now) {
    sampler_->update(now);
    return {MetricType::NetDownload, sampler_->downloadKBs(), now, sampler_->valid()};
}

}  // namespace glance::android
