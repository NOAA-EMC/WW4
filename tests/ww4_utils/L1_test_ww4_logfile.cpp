/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_ww4_logfile.cpp
 * @brief Unit tests for log file output routines.
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
#include "ww4_utils/ww4_logfile.h"
#include <gtest/gtest.h>
#include <sstream>

using namespace ww4_utils::ww4_logfile;
using ww4_utils::DateTime;

TEST(LogFileTest, InitialOutput) {
  std::stringstream ss;
  writeInitialOutput(ss, "Program shell");
  std::string output = ss.str();

  EXPECT_NE(output.find("*** WAVEWATCH IV program Program shell ***"),
            std::string::npos);
}

TEST(LogFileTest, FinalOutputWithMetrics) {
  std::stringstream ss;
  writeFinalOutput(ss, "Program shell", 1.23, 4.56);
  std::string output = ss.str();

  EXPECT_NE(output.find("  Initialization time :      1.23 s"),
            std::string::npos);
  EXPECT_NE(output.find("  Elapsed time       4.56 s"), std::string::npos);
  EXPECT_EQ(output.find("  Memory usage:"), std::string::npos);
  EXPECT_NE(output.find("  End of program "), std::string::npos);
  EXPECT_NE(output.find("WAVEWATCH IV program Program shell"),
            std::string::npos);
}

TEST(LogFileTest, LogTableDataOperations) {
  LogTableData data;
  EXPECT_FALSE(data.anyAction());

  data.wlUpdated = true;
  EXPECT_TRUE(data.anyAction());

  data.reset();
  EXPECT_FALSE(data.anyAction());

  data.cuUpdated = true;
  EXPECT_TRUE(data.anyAction());

  data.reset();
  data.wiUpdated = true;
  EXPECT_TRUE(data.anyAction());

  data.reset();
  data.icUpdated = true;
  EXPECT_TRUE(data.anyAction());

  data.reset();
  data.bdUpdated = true;
  EXPECT_TRUE(data.anyAction());

  data.reset();
  data.fieldsPerformed = true;
  EXPECT_TRUE(data.anyAction());

  data.reset();
  data.pointsPerformed = true;
  EXPECT_TRUE(data.anyAction());

  data.reset();
  data.restartPerformed = true;
  EXPECT_TRUE(data.anyAction());

  data.reset();
  data.apiPerformed = true;
  EXPECT_TRUE(data.anyAction());

  data.reset();
  EXPECT_FALSE(data.anyAction());
}

TEST(LogFileTest, WriteUpdatingFieldAndInterpolation) {
  std::stringstream ss;
  writeUpdatingField(ss, "winds");
  std::string output = ss.str();
  EXPECT_NE(output.find("    Updating winds"), std::string::npos);

  DateTime t1{20260101, 0.0};
  DateTime t2{20260101, 10000.0};
  std::stringstream ss2;
  writeInterpolationInfo(ss2, t1, t2);
  std::string output2 = ss2.str();
  EXPECT_NE(output2.find("      Interpolation from"), std::string::npos);
}

TEST(LogFileTest, TabularLogTable) {
  std::stringstream ss;
  writeLogTableHeader(ss);
  std::string output = ss.str();
  EXPECT_NE(output.find("Inputs"), std::string::npos);
  EXPECT_NE(output.find("Outputs"), std::string::npos);

  DateTime t{20260101, 0.0};
  LogTableData data;
  data.wlUpdated = true;
  data.apiPerformed = true;

  std::stringstream ssLine;
  writeLogTableLine(ssLine, t, data);
  std::string lineOutput = ssLine.str();
  EXPECT_NE(lineOutput.find("X"), std::string::npos);
  EXPECT_NE(lineOutput.find("|"), std::string::npos);

  std::stringstream ssFooter;
  writeLogTableFooter(ssFooter);
  std::string footerOutput = ssFooter.str();
  EXPECT_NE(footerOutput.find("+"), std::string::npos);
}
