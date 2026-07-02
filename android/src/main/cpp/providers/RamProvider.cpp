#include "providers/RamProvider.hpp"

#include <unistd.h>

#include <cstdint>
#include <cstdio>

namespace glance::android {

MetricSample RamProvider::sample(MetricSample::TimePoint now) {
    std::FILE* file = std::fopen("/proc/self/statm", "re");
    if (file == nullptr) {
        return {MetricType::RamUsage, 0.0, now, false};
    }

    unsigned long long totalPages = 0;
    unsigned long long residentPages = 0;
    const int matched = std::fscanf(file, "%llu %llu", &totalPages, &residentPages);
    std::fclose(file);

    if (matched != 2) {
        return {MetricType::RamUsage, 0.0, now, false};
    }

    const long pageSize = ::sysconf(_SC_PAGESIZE);
    if (pageSize <= 0) {
        return {MetricType::RamUsage, 0.0, now, false};
    }

    const double bytes =
        static_cast<double>(residentPages) * static_cast<double>(pageSize);
    const double megabytes = bytes / (1024.0 * 1024.0);
    return {MetricType::RamUsage, megabytes, now, true};
}

}  // namespace glance::android
