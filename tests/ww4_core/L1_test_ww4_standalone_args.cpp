/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_ww4_standalone_args.cpp
 * @brief Unit tests for stand-alone command line argument parsing.
 * @details This file provides unit tests for parseStandaloneArgs and
 *          reportStandaloneArgs in ww4_standalone_args.cpp.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-22
 * @date Last update : 2026-10-22
 */

#include "ww4_progs/ww4_standalone_args.h"
#include <gtest/gtest.h>
#include <sstream>
#include <vector>

using namespace ww4_progs;

TEST(StandaloneArgsTest, ValidFourArguments) {
  std::vector<char *> argv = {
      const_cast<char *>("ww4_standalone"), const_cast<char *>("20260101"),
      const_cast<char *>("000000"), const_cast<char *>("20260101"),
      const_cast<char *>("020000")};
  int argc = static_cast<int>(argv.size());

  std::stringstream ss;
  const auto args = parseStandaloneArgs(argc, argv.data(), ss);
  ASSERT_TRUE(args.has_value());
  EXPECT_EQ(args->startTime.ymd, 20260101);
  EXPECT_NEAR(args->startTime.hms, 000000.0, 1e-6);
  EXPECT_EQ(args->endTime.ymd, 20260101);
  EXPECT_NEAR(args->endTime.hms, 020000.0, 1e-6);
}

TEST(StandaloneArgsTest, ValidTwoArguments) {
  std::vector<char *> argv = {const_cast<char *>("ww4_standalone"),
                              const_cast<char *>("19680606 060000"),
                              const_cast<char *>("19680606 180000")};
  int argc = static_cast<int>(argv.size());

  std::stringstream ss;
  const auto args = parseStandaloneArgs(argc, argv.data(), ss);
  ASSERT_TRUE(args.has_value());
  EXPECT_EQ(args->startTime.ymd, 19680606);
  EXPECT_NEAR(args->startTime.hms, 60000.0, 1e-6);
  EXPECT_EQ(args->endTime.ymd, 19680606);
  EXPECT_NEAR(args->endTime.hms, 180000.0, 1e-6);
}

TEST(StandaloneArgsTest, ValidFlags) {
  std::vector<char *> argv = {const_cast<char *>("ww4_standalone"),
                              const_cast<char *>("-s"),
                              const_cast<char *>("20260101"),
                              const_cast<char *>("000000"),
                              const_cast<char *>("-e"),
                              const_cast<char *>("20260101"),
                              const_cast<char *>("020000")};
  int argc = static_cast<int>(argv.size());

  std::stringstream ss;
  const auto args = parseStandaloneArgs(argc, argv.data(), ss);
  ASSERT_TRUE(args.has_value());
  EXPECT_EQ(args->startTime.ymd, 20260101);
  EXPECT_EQ(args->endTime.ymd, 20260101);
}

TEST(StandaloneArgsTest, MissingArguments) {
  std::vector<char *> argv = {const_cast<char *>("ww4_standalone")};
  int argc = static_cast<int>(argv.size());

  std::stringstream ss;
  const auto args = parseStandaloneArgs(argc, argv.data(), ss);
  EXPECT_FALSE(args.has_value());
}

TEST(StandaloneArgsTest, HelpFlag) {
  std::vector<char *> argv = {const_cast<char *>("ww4_standalone"),
                              const_cast<char *>("--help")};
  int argc = static_cast<int>(argv.size());

  std::stringstream ss;
  const auto args = parseStandaloneArgs(argc, argv.data(), ss);
  EXPECT_FALSE(args.has_value());
}

TEST(StandaloneArgsTest, InvalidTimeOrder) {
  std::vector<char *> argv = {const_cast<char *>("ww4_standalone"),
                              const_cast<char *>("20260102 000000"),
                              const_cast<char *>("20260101 000000")};
  int argc = static_cast<int>(argv.size());

  std::stringstream ss;
  const auto args = parseStandaloneArgs(argc, argv.data(), ss);
  EXPECT_FALSE(args.has_value());
}

TEST(StandaloneArgsTest, MalformedDateTime) {
  std::vector<char *> argv = {const_cast<char *>("ww4_standalone"),
                              const_cast<char *>("invalid_time"),
                              const_cast<char *>("20260101 020000")};
  int argc = static_cast<int>(argv.size());

  std::stringstream ss;
  const auto args = parseStandaloneArgs(argc, argv.data(), ss);
  EXPECT_FALSE(args.has_value());
}

TEST(StandaloneArgsTest, ReportArgs) {
  StandaloneArgs args;
  args.startTime = {19680606, 60000.0};
  args.endTime = {19680606, 180000.0};

  std::stringstream ss;
  reportStandaloneArgs(args, ss);
  std::string output = ss.str();

  EXPECT_NE(output.find("Stand-alone configuration settings :"),
            std::string::npos);
  EXPECT_NE(output.find("Start time         : 1968/06/06 06:00:00 UTC"),
            std::string::npos);
  EXPECT_NE(output.find("End time           : 1968/06/06 18:00:00 UTC"),
            std::string::npos);
}
