/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_ww4_std_out.cpp
 * @brief Unit tests for standard screen output routines.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-07-09
 * @date Last update : 2026-07-09
 */

#include "ww4_utils/time_management.h"
#include "ww4_utils/ww4_std_out.h"
#include <gtest/gtest.h>
#include <sstream>

using namespace ww4_utils::ww4_std_out;
using ww4_utils::DateTime;
using ww4_utils::MemoryUsage;

TEST(StdOutTest, InitialOutputProgramShell) {
  std::stringstream ss;
  writeInitialOutput(ss, "Program shell");
  std::string output = ss.str();

  // Expected 15 spaces then banner
  EXPECT_NE(
      output.find("      *** WAVEWATCH IV program Program shell ***      "),
      std::string::npos);
  EXPECT_NE(
      output.find("======================================================"),
      std::string::npos);
}

TEST(StdOutTest, FinalOutputProgramShellWithMetrics) {
  std::stringstream ss;
  writeFinalOutput(ss, "Program shell", 1.23, 4.56);
  std::string output = ss.str();

  EXPECT_NE(output.find("  Initialization time :      1.23 s"),
            std::string::npos);
  EXPECT_NE(output.find("  Elapsed time       4.56 s"), std::string::npos);
  EXPECT_EQ(output.find("  Memory usage:"), std::string::npos);
  EXPECT_NE(output.find("  End of program "), std::string::npos);
  EXPECT_NE(output.find("          WAVEWATCH IV program Program shell "),
            std::string::npos);
}

TEST(StdOutTest, FinalOutputWithoutMetrics) {
  std::stringstream ss;
  writeFinalOutput(ss, "Program shell");
  std::string output = ss.str();

  EXPECT_EQ(output.find("Initialization time"), std::string::npos);
  EXPECT_EQ(output.find("Memory usage"), std::string::npos);
  EXPECT_NE(output.find("  End of program "), std::string::npos);
}

TEST(StdOutTest, WriteExtcdeOutputFormatting) {
  std::stringstream ss;
  writeExtcdeOutput(ss, "Fatal error", "main.cpp", 42);
  std::string output = ss.str();

  EXPECT_NE(output.find("WW4 ERROR: Fatal error"), std::string::npos);
  EXPECT_NE(output.find("WW4 ERROR: FILE=main.cpp LINE=42"), std::string::npos);
}

TEST(StdOutTest, WriteExtcdeOutputOnlyMessage) {
  std::stringstream ss;
  writeExtcdeOutput(ss, "Simple error");
  std::string output = ss.str();

  EXPECT_NE(output.find("WW4 ERROR: Simple error"), std::string::npos);
  EXPECT_EQ(output.find("FILE="), std::string::npos);
}

TEST(StdOutTest, ExtcdeTermination) {
  const int expectedExitCode = 1;
  const std::string errorMsg = "Fatal program error";

  // Use EXPECT_EXIT to verify that extcde calls std::exit with the correct code
  // and prints the expected message to stderr (default for extcde).
  EXPECT_EXIT(
      extcde(expectedExitCode, std::cerr, errorMsg, "test.cpp", 123),
      ::testing::ExitedWithCode(expectedExitCode),
      "WW4 ERROR: Fatal program error.*WW4 ERROR: FILE=test.cpp LINE=123");
}

TEST(StdOutTest, WriteWarnngOutputFormatting) {
  std::stringstream ss;
  writeWarnngOutput(ss, "Warning message", "main.cpp", 42);
  std::string output = ss.str();

  EXPECT_NE(output.find("WW4 WARNING: Warning message"), std::string::npos);
  EXPECT_NE(output.find("WW4 WARNING: FILE=main.cpp LINE=42"),
            std::string::npos);
}

TEST(StdOutTest, WarnngReporting) {
  std::stringstream ss;
  warnng(ss, "Another warning");
  std::string output = ss.str();

  EXPECT_NE(output.find("WW4 WARNING: Another warning"), std::string::npos);
}

TEST(StdOutTest, WriteUpdatingFieldAndInterpolation) {
  std::stringstream ss;
  writeUpdatingField(ss, "currents");
  std::string output = ss.str();
  EXPECT_NE(output.find("    Updating currents"), std::string::npos);

  DateTime t1{20260101, 0.0};
  DateTime t2{20260101, 10000.0};
  std::stringstream ss2;
  writeInterpolationInfo(ss2, t1, t2);
  std::string output2 = ss2.str();
  EXPECT_NE(output2.find("      Interpolation from"), std::string::npos);
}
