/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L2_test_w4core_wave_output.cpp
 * @brief Unit tests for w4core_wave output.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Last update : 2026-06-18
 */

#include "ww4_core/w4core_finalize.h"
#include "ww4_core/w4core_init.h"
#include "ww4_core/w4core_wave.h"
#include "ww4_utils/time_management.h"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>

class W4CoreWaveOutputTest : public ::testing::Test {
protected:
  void SetUp() override {
    startTime = {19680606, 0.0};
    endTime = {19680606, 3600.0};
  }

  void TearDown() override {
    ww4_core::resetInternalState();
    std::remove("ww4_run_config.yaml");
    std::remove("ww4_log.txt");
  }

  ww4_utils::DateTime startTime;
  ww4_utils::DateTime endTime;
};

TEST_F(W4CoreWaveOutputTest, ReportsApiOutputStatus) {
  // Test Case 1: API output disabled (default)
  {
    std::ofstream runFile("ww4_run_config.yaml");
    runFile << "general:\n";
    runFile << "  calendar_type: Standard\n";
    runFile << "  time_step: 3600.0\n";
    runFile << "physics:\n";
    runFile << "  solver: UQ\n";
    runFile << "  linear_input: LN1\n";
    runFile << "  input_dissipation: ST4\n";
    runFile << "  nonlinear_interactions: NL1\n";
    runFile << "  bottom_friction: BT1\n";
    runFile << "forcing:\n";
    runFile << "  water_levels: none\n";
    runFile << "  currents: none\n";
    runFile << "  winds: none\n";
    runFile << "  ice_concentrations: none\n";
    runFile << "  bottom_depth: none\n";
    runFile << "output:\n";
    runFile << "  api:\n";
    runFile << "    requested: no\n";

    runFile.close();

    std::stringstream ss;
    ww4_core::w4core_init(startTime, "test_program", ss);
    ww4_core::w4core_wave(startTime, endTime, ss);

    std::string output = ss.str();
    EXPECT_EQ(output.find("Performing API output"), std::string::npos);
    EXPECT_NE(output.find("API output not requested"), std::string::npos);
  }

  ww4_core::resetInternalState();

  // Test Case 2: API output enabled
  {
    std::ofstream runFile("ww4_run_config.yaml");
    runFile << "general:\n";
    runFile << "  calendar_type: Standard\n";
    runFile << "  time_step: 3600.0\n";
    runFile << "physics:\n";
    runFile << "  solver: UQ\n";
    runFile << "  linear_input: LN1\n";
    runFile << "  input_dissipation: ST4\n";
    runFile << "  nonlinear_interactions: NL1\n";
    runFile << "  bottom_friction: BT1\n";
    runFile << "forcing:\n";
    runFile << "  water_levels: none\n";
    runFile << "  currents: none\n";
    runFile << "  winds: none\n";
    runFile << "  ice_concentrations: none\n";
    runFile << "  bottom_depth: none\n";
    runFile << "output:\n";
    runFile << "  api:\n";
    runFile << "    requested: yes\n";

    runFile.close();

    std::stringstream ss;
    ww4_core::w4core_init(startTime, "test_program", ss);
    ww4_core::w4core_wave(startTime, endTime, ss);

    std::string output = ss.str();
    EXPECT_NE(output.find("Performing API output"), std::string::npos);
    EXPECT_NE(output.find("API output"), std::string::npos);
  }
}

TEST_F(W4CoreWaveOutputTest, DoesNotReportBottomDepthFromGrid) {
  std::ofstream runFile("ww4_run_config.yaml");
  runFile << "general:\n";
  runFile << "  calendar_type: Standard\n";
  runFile << "  time_step: 3600.0\n";
  runFile << "physics:\n";
  runFile << "  solver: UQ\n";
  runFile << "  linear_input: LN1\n";
  runFile << "  input_dissipation: ST4\n";
  runFile << "  nonlinear_interactions: NL1\n";
  runFile << "  bottom_friction: BT1\n";
  runFile << "forcing:\n";
  runFile << "  water_levels: none\n";
  runFile << "  currents: none\n";
  runFile << "  winds: none\n";
  runFile << "  ice_concentrations: none\n";
  runFile << "  bottom_depth: from_grid\n";

  runFile.close();

  std::stringstream ss;
  ww4_core::w4core_init(startTime, "test_program", ss);
  ww4_core::w4core_wave(startTime, endTime, ss);

  std::string output = ss.str();
  // It should NOT contain "Updating bottom depth"
  EXPECT_EQ(output.find("Updating bottom depth"), std::string::npos);
}
