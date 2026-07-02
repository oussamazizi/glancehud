/// @file NetProvider.hpp
/// @brief Android/Linux network throughput providers (read /proc/net/dev).
#ifndef GLANCEHUD_ANDROID_NET_PROVIDER_HPP
#define GLANCEHUD_ANDROID_NET_PROVIDER_HPP

#include <cstdint>
#include <memory>

#include "glancehud/IMetricProvider.hpp"

namespace glance::android {

/// @brief Shared reader of total device network counters.
///
/// A single provider maps to a single metric, but network has two (up/down).
/// So the two providers below share one @ref NetSampler, which reads
/// /proc/net/dev at most once per sampling tick and exposes both rates.
class NetSampler {
public:
    /// @brief Refresh rates for tick @p now (idempotent within the same tick).
    void update(MetricSample::TimePoint now);

    [[nodiscard]] double uploadKBs() const noexcept { return uploadKBs_; }
    [[nodiscard]] double downloadKBs() const noexcept { return downloadKBs_; }
    [[nodiscard]] bool valid() const noexcept { return valid_; }

private:
    bool haveBaseline_{false};
    bool valid_{false};
    std::uint64_t lastRx_{0};
    std::uint64_t lastTx_{0};
    MetricSample::TimePoint lastNow_{};

    bool updatedForValid_{false};
    MetricSample::TimePoint updatedFor_{};

    double uploadKBs_{0.0};
    double downloadKBs_{0.0};
};

/// @brief Upload throughput in KB/s (device total).
class NetUploadProvider final : public IMetricProvider {
public:
    explicit NetUploadProvider(std::shared_ptr<NetSampler> sampler)
        : sampler_(std::move(sampler)) {}

    [[nodiscard]] MetricType type() const noexcept override { return MetricType::NetUpload; }
    [[nodiscard]] MetricSample sample(MetricSample::TimePoint now) override;

private:
    std::shared_ptr<NetSampler> sampler_;
};

/// @brief Download throughput in KB/s (device total).
class NetDownloadProvider final : public IMetricProvider {
public:
    explicit NetDownloadProvider(std::shared_ptr<NetSampler> sampler)
        : sampler_(std::move(sampler)) {}

    [[nodiscard]] MetricType type() const noexcept override { return MetricType::NetDownload; }
    [[nodiscard]] MetricSample sample(MetricSample::TimePoint now) override;

private:
    std::shared_ptr<NetSampler> sampler_;
};

}  // namespace glance::android

#endif  // GLANCEHUD_ANDROID_NET_PROVIDER_HPP
