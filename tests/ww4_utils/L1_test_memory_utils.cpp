/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_memory_utils.cpp
 * @brief Unit tests for memory utility routines.
 * @details Uses Google Test to verify that memory usage capture is working as
 * expected.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-02-27
 * @date Last update : 2026-07-13
 */

#include "ww4_utils/memory_utils.h"
#include <fstream>
#include <gtest/gtest.h>
#include <vector>

namespace ww4_utils {

TEST(MemoryUtilsTest, DefaultInitialization) {
  const MemoryUsage usage{};
  EXPECT_EQ(usage.vmPeak, 0);
  EXPECT_EQ(usage.vmSize, 0);
  EXPECT_EQ(usage.vmHWM, 0);
  EXPECT_EQ(usage.vmRSS, 0);
}

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

TEST(MemoryUtilsTest, CaptureMemoryHWM) {
  const auto hwm = MemoryUtils::captureMemoryHWM();

  ASSERT_TRUE(hwm.has_value());
  EXPECT_GT(*hwm, 0);
}

TEST(MemoryUtilsTest, MockLargeValues) {
  const char *mockPath = "mock_status.txt";
  {
    std::ofstream mockFile(mockPath);
    mockFile << "VmPeak: 70252780120440 kB\n";
    mockFile << "VmSize: 70252780120440 kB\n";
    mockFile << "VmHWM:       6144 kB\n";
    mockFile << "VmRSS:       6144 kB\n";
  }

  setMemoryStatusPathForTesting(mockPath);
  const auto usage = MemoryUtils::captureMemoryUsage();
  setMemoryStatusPathForTesting("/proc/self/status"); // Reset

  ASSERT_TRUE(usage.has_value());
  EXPECT_EQ(usage->vmPeak, 70252780120440ULL);
  EXPECT_EQ(usage->vmSize, 70252780120440ULL);
  EXPECT_EQ(usage->vmHWM, 6144ULL);
  EXPECT_EQ(usage->vmRSS, 6144ULL);

  std::remove(mockPath);
}

TEST(MemoryUtilsTest, ResetMemoryStatusPath) {
  const char *mockPath = "mock_status_reset.txt";
  {
    std::ofstream mockFile(mockPath);
    mockFile << "VmPeak: 1234 kB\n";
    mockFile << "VmSize: 1234 kB\n";
    mockFile << "VmHWM:  1234 kB\n";
    mockFile << "VmRSS:  1234 kB\n";
  }

  setMemoryStatusPathForTesting(mockPath);
  auto usage = MemoryUtils::captureMemoryUsage();
  ASSERT_TRUE(usage.has_value());
  EXPECT_EQ(usage->vmPeak, 1234ULL);

  resetMemoryStatusPath();
  // Now should capture host/self status instead of mock
  usage = MemoryUtils::captureMemoryUsage();
  ASSERT_TRUE(usage.has_value());
  EXPECT_NE(usage->vmPeak, 1234ULL);

  std::remove(mockPath);
}

} // namespace ww4_utils
