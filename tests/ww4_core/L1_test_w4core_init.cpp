/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_w4core_init.cpp
 * @brief Unit tests for the WAVEWATCH IV core initialization.
 * @details This file tests initialization and state management in ww4_core.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-07-09
 * @date Last update : 2026-07-13
 */

#include "ww4_core/w4core_init.h"
#include <fstream>
#include <gtest/gtest.h>

namespace ww4_core {

class W4CoreInitTest : public ::testing::Test {
protected:
  void SetUp() override { resetInternalState(); }

  void TearDown() override {
    resetInternalState();
    std::remove("ww4_run_config.yaml");
    std::remove("ww4_log.txt");
  }
};

TEST_F(W4CoreInitTest, ResetInternalStateAndAccessors) {
  resetInternalState();
  EXPECT_EQ(getProgramName(), "");
  EXPECT_EQ(getRunConfig().timeStep, -1.0);
  EXPECT_FALSE(getLogFileStream().is_open());
}

TEST_F(W4CoreInitTest, UpdateWaveModelAndInputTime) {
  ww4_utils::DateTime t = {20260101, 120000.0};
  updateWaveModelTime(t);
  EXPECT_EQ(*getWaveTimeData().modelTime, t);

  ww4_utils::intTimeData inputTime{};
  inputTime.time1 = t;
  updateWaveInputTime(ww4_utils::InputType::Winds, inputTime);
  EXPECT_EQ(getWaveTimeData().winds.time1, t);

  updateWaveInputTime(ww4_utils::InputType::WaterLevels, inputTime);
  EXPECT_EQ(getWaveTimeData().waterLevels.time1, t);

  updateWaveInputTime(ww4_utils::InputType::Currents, inputTime);
  EXPECT_EQ(getWaveTimeData().currents.time1, t);

  updateWaveInputTime(ww4_utils::InputType::IceConcentrations, inputTime);
  EXPECT_EQ(getWaveTimeData().iceConcentrations.time1, t);

  updateWaveInputTime(ww4_utils::InputType::BottomDepth, inputTime);
  EXPECT_EQ(getWaveTimeData().bottomDepth.time1, t);
}

TEST_F(W4CoreInitTest, MutableRunConfig) {
  getMutableRunConfig().timeStep = 7200.0;
  EXPECT_EQ(getRunConfig().timeStep, 7200.0);
}

TEST_F(W4CoreInitTest, W4CoreInitAndReset) {
  // Create dummy run configuration file
  std::ofstream runFile("ww4_run_config.yaml");
  runFile << "general:\n";
  runFile << "  calendar_type: Standard\n";
  runFile << "  time_step: 3600.0\n";
  runFile << "forcing:\n";
  runFile << "  water_levels: none\n";
  runFile << "  currents: none\n";
  runFile << "  winds: none\n";
  runFile << "  ice_concentrations: none\n";
  runFile << "  bottom_depth: none\n";
  runFile.close();

  ww4_utils::DateTime startTime = {20260101, 0.0};
  w4core_init(startTime, "test_init", std::cout);

  EXPECT_EQ(getProgramName(), "test_init");
  EXPECT_EQ(getRunConfig().timeStep, 3600.0);
}

} // namespace ww4_core
