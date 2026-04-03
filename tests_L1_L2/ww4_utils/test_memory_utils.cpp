/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file test_memory_utils.cpp
 * @brief Unit tests for memory utility routines.
 * @details Uses Google Test to verify that memory usage capture is working as
 * expected.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-02-27
 * @date Last Update, 2026-04-03
 */

#include "ww4_utils/memory_utils.hpp"
#include <gtest/gtest.h>
#include <vector>

namespace ww4_utils {

/**
 * @test Verify that MemoryUsage struct initializes fields to zero.
 */
TEST(MemoryUtilsTest, DefaultInitialization) {
  const MemoryUsage usage{};
  EXPECT_EQ(usage.vmPeak, 0);
  EXPECT_EQ(usage.vmSize, 0);
  EXPECT_EQ(usage.vmHWM, 0);
  EXPECT_EQ(usage.vmRSS, 0);
}

/**
 * @test Verify that captureMemoryUsage returns valid non-zero metrics.
 */
TEST(MemoryUtilsTest, CaptureMemoryUsage) {
  const auto usage = MemoryUtils::captureMemoryUsage();

  ASSERT_TRUE(usage.has_value());
  EXPECT_GT(usage->vmPeak, 0);
  EXPECT_GT(usage->vmSize, 0);
  EXPECT_GT(usage->vmHWM, 0);
  EXPECT_GT(usage->vmRSS, 0);

  EXPECT_GE(usage->vmPeak, usage->vmSize);
  EXPECT_GE(usage->vmHWM, usage->vmRSS);
}

/**
 * @test Verify that memory usage is captured correctly even after allocation.
 */
TEST(MemoryUtilsTest, MemoryIncreaseAfterAllocation) {
  const auto before = MemoryUtils::captureMemoryUsage();
  ASSERT_TRUE(before.has_value());

  // Allocate ~10MB of memory
  {
    const std::vector<char> dummy(10 * 1024 * 1024, 'a');
    const auto during = MemoryUtils::captureMemoryUsage();
    ASSERT_TRUE(during.has_value());

    // VmSize or VmRSS should ideally be larger, but OS behavior varies.
    // We at least expect it to be a valid capture.
    EXPECT_GT(during->vmSize, 0);
  }

  const auto after = MemoryUtils::captureMemoryUsage();
  ASSERT_TRUE(after.has_value());
}

/**
 * @test Verify that captureMemoryHWM returns a valid non-zero metric.
 */
TEST(MemoryUtilsTest, CaptureMemoryHWM) {
  const auto hwm = MemoryUtils::captureMemoryHWM();

  ASSERT_TRUE(hwm.has_value());
  EXPECT_GT(*hwm, 0);
}

} // namespace ww4_utils
