/// @file smoke_test.cpp
/// @brief Sanity checks that the public contracts compile and behave.
#include <gtest/gtest.h>

#include "glancehud/MetricType.hpp"
#include "glancehud/Snapshot.hpp"
#include "glancehud/Version.hpp"

namespace {

TEST(Version, MatchesConstants) {
    EXPECT_EQ(glance::version(), "0.1.0");
    EXPECT_EQ(glance::kVersionMajor, 0);
}

TEST(MetricType, LabelsAndUnits) {
    EXPECT_EQ(glance::toLabel(glance::MetricType::Fps), "FPS");
    EXPECT_EQ(glance::toUnit(glance::MetricType::RamUsage), "MB");
    EXPECT_EQ(glance::kMetricTypeCount, 7u);
}

TEST(Snapshot, IsDefaultConstructibleAndIndexable) {
    glance::Snapshot snap{};
    EXPECT_EQ(snap.sequence, 0u);
    EXPECT_FALSE(snap[glance::MetricType::CpuUsage].valid);
}

}  // namespace
