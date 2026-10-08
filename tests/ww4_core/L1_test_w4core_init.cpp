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
 * @date Last update : 2026-09-29
 */

#include "ww4_core/solver_smc/solver_smc.h"
#include "ww4_core/solver_triangular/solver_triangular.h"
#include "ww4_core/solver_uq/solver_uq.h"
#include "ww4_core/w4core_init.h"
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

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
  runFile << "spectral_space:\n";
  runFile << "  num_directions: 36\n";
  runFile << "  num_frequencies: 50\n";
  runFile << "  freq_increment_factor: 1.07\n";
  runFile << "  first_frequency: 0.035\n";
  runFile << "  first_direction_offset: 0.5\n";
  runFile.close();

  ww4_utils::DateTime startTime = {20260101, 0.0};
  w4core_init(startTime, "test_init", std::cout);

  EXPECT_EQ(getProgramName(), "test_init");
  EXPECT_EQ(getRunConfig().timeStep, 3600.0);
  EXPECT_EQ(getRunConfig().spectralSpace.numDirections, 36);
  EXPECT_EQ(getRunConfig().spectralSpace.numFrequencies, 50);
  EXPECT_DOUBLE_EQ(getRunConfig().spectralSpace.freqIncrementFactor, 1.07);
  EXPECT_DOUBLE_EQ(getRunConfig().spectralSpace.firstFrequency, 0.035);
  EXPECT_DOUBLE_EQ(getRunConfig().spectralSpace.firstDirectionOffset, 0.5);

  resetInternalState();
  EXPECT_EQ(getProgramName(), "");
}

TEST_F(W4CoreInitTest, SolverInitRoutines) {
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  w4core_init_uq(ss);
  EXPECT_NE(ss.str().find("Initializing UQ regular grid solver"),
            std::string::npos);

  ss.str("");
  w4core_init_triangular(ss);
  EXPECT_NE(ss.str().find("Initializing Triangular unstructured grid solver"),
            std::string::npos);

  ss.str("");
  w4core_init_smc(ss);
  EXPECT_NE(ss.str().find("Initializing SMC grid solver"), std::string::npos);

  SolverRectangularGrid uq;
  uq.init();

  SolverTriangularGrid triangular;
  triangular.init();

  SolverSMCGrid smc;
  smc.init();
}

TEST_F(W4CoreInitTest, InitWithTriangularAndSMC) {
  std::ofstream runFile("ww4_run_config.yaml");
  runFile << "general:\n";
  runFile << "  calendar_type: Standard\n";
  runFile << "  time_step: 3600.0\n";
  runFile << "physics:\n";
  runFile << "  solver: triangular\n";
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
  runFile.close();

  ww4_utils::DateTime startTime = {20260101, 0.0};
  std::ostringstream ss;
  w4core_init(startTime, "test_init", ss);

  EXPECT_EQ(getProgramName(), "test_init");

  resetInternalState();

  std::ofstream runFile2("ww4_run_config.yaml");
  runFile2 << "general:\n";
  runFile2 << "  calendar_type: Standard\n";
  runFile2 << "  time_step: 3600.0\n";
  runFile2 << "physics:\n";
  runFile2 << "  solver: SMC\n";
  runFile2 << "  linear_input: LN1\n";
  runFile2 << "  input_dissipation: ST4\n";
  runFile2 << "  nonlinear_interactions: NL1\n";
  runFile2 << "  bottom_friction: BT1\n";
  runFile2 << "forcing:\n";
  runFile2 << "  water_levels: none\n";
  runFile2 << "  currents: none\n";
  runFile2 << "  winds: none\n";
  runFile2 << "  ice_concentrations: none\n";
  runFile2 << "  bottom_depth: none\n";
  runFile2.close();

  w4core_init(startTime, "test_init_smc", ss);
  EXPECT_EQ(getProgramName(), "test_init_smc");
}

} // namespace ww4_core
