#include "providers/CpuProvider.hpp"

#include <unistd.h>

#include <cstdio>
#include <cstdint>

namespace glance::android {
namespace {

/// Sum of the process's user + kernel jiffies from /proc/self/stat.
/// Fields 14 (utime) and 15 (stime) are the 14th/15th whitespace-separated
/// tokens after the (parenthesised) comm field.
bool readProcessJiffies(std::uint64_t& out) {
    std::FILE* file = std::fopen("/proc/self/stat", "re");
    if (file == nullptr) {
        return false;
    }

    // Skip up to and including the ')' that closes the comm field, which may
    // itself contain spaces or digits.
    int ch = 0;
    while ((ch = std::fgetc(file)) != EOF && ch != ')') {
    }

    unsigned long long utime = 0;
    unsigned long long stime = 0;
    // After ')' come: ' S' then the numeric fields. utime is field 14, stime 15;
    // relative to here (field 3 = state) they are the 12th and 13th tokens.
    const int matched = std::fscanf(
        file,
        " %*c"                                   // state
        " %*d %*d %*d %*d %*d"                    // ppid pgrp session tty tpgid
        " %*u %*u %*u %*u %*u"                    // flags minflt cminflt majflt cmajflt
        " %llu %llu",                            // utime stime
        &utime, &stime);
    std::fclose(file);

    if (matched != 2) {
        return false;
    }
    out = static_cast<std::uint64_t>(utime) + static_cast<std::uint64_t>(stime);
    return true;
}

/// Sum of all jiffies on the first ("cpu") line of /proc/stat.
bool readTotalJiffies(std::uint64_t& out) {
    std::FILE* file = std::fopen("/proc/stat", "re");
    if (file == nullptr) {
        return false;
    }

    char label[8] = {0};
    if (std::fscanf(file, "%7s", label) != 1) {
        std::fclose(file);
        return false;
    }

    std::uint64_t total = 0;
    unsigned long long field = 0;
    while (std::fscanf(file, "%llu", &field) == 1) {
        total += static_cast<std::uint64_t>(field);
    }
    std::fclose(file);

    out = total;
    return total > 0;
}

}  // namespace

MetricSample CpuProvider::sample(MetricSample::TimePoint now) {
    std::uint64_t process = 0;
    std::uint64_t total = 0;
    if (!readProcessJiffies(process) || !readTotalJiffies(total)) {
        return {MetricType::CpuUsage, 0.0, now, false};
    }

    if (!havePrevious_) {
        lastProcess_ = process;
        lastTotal_ = total;
        havePrevious_ = true;
        return {MetricType::CpuUsage, 0.0, now, false};  // Need two samples.
    }

    const std::uint64_t deltaProcess = process - lastProcess_;
    const std::uint64_t deltaTotal = total - lastTotal_;
    lastProcess_ = process;
    lastTotal_ = total;

    if (deltaTotal == 0) {
        return {MetricType::CpuUsage, 0.0, now, false};
    }

    const long cores = ::sysconf(_SC_NPROCESSORS_ONLN);
    const double numCores = cores > 0 ? static_cast<double>(cores) : 1.0;

    // Fraction of total system CPU time consumed by this process, scaled so the
    // value reads as a percentage of a single core (may exceed 100 on load).
    const double percent = (static_cast<double>(deltaProcess) /
                            static_cast<double>(deltaTotal)) *
                           numCores * 100.0;

    return {MetricType::CpuUsage, percent, now, true};
}

}  // namespace glance::android
