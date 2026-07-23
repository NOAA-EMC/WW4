/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_w4core_wave.cpp
 * @brief Unit tests for the WAVEWATCH IV core wave time-stepping loop.
 * @details This file tests the w4core_wave routine.
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
#include "ww4_core/w4core_wave.h"
#include <fstream>
#include <gtest/gtest.h>

namespace ww4_core {

class W4CoreWaveL1Test : public ::testing::Test {
protected:
  void SetUp() override { resetInternalState(); }

  void TearDown() override {
    resetInternalState();
    std::remove("ww4_run_config.yaml");
    std::remove("ww4_log.txt");
  }
};

TEST_F(W4CoreWaveL1Test, BasicWaveExecution) {
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
  w4core_init(startTime, "test_wave", std::cout);

  ww4_utils::DateTime endTime = {20260101,
                                 20000.0}; // 2 hours (2 * 3600 seconds)
  EXPECT_NO_THROW(w4core_wave(startTime, endTime, std::cout));
  EXPECT_EQ(*getWaveTimeData().modelTime, endTime);
}

} // namespace ww4_core
