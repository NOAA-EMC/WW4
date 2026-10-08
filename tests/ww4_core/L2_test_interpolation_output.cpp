/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L2_test_interpolation_output.cpp
 * @brief Unit tests to verify that "Interpolation from" is only printed when
 * updated.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Contributors: Jules (Agentic AI)
 * @date 2026-05-01
 * @date Last update : 2026-06-18
 */

#include "ww4_core/w4core_finalize.h"
#include "ww4_core/w4core_init.h"
#include "ww4_core/w4core_wave.h"
#include "ww4_utils/time_management.h"
#include <fstream>
#include <gtest/gtest.h>
#include <regex>
#include <sstream>
#include <string>

class InterpolationOutputTest : public ::testing::Test {
protected:
  void SetUp() override {
    ww4_core::resetInternalState();
    ww4_utils::TimeManagement::reset();
  }

  void TearDown() override {
    ww4_core::resetInternalState();
    ww4_utils::TimeManagement::reset();
    std::remove("ww4_run_config.yaml");
    std::remove("ww4_log.txt");
  }

  void writeYaml(const std::string &content) {
    std::ofstream ofs("ww4_run_config.yaml");
    ofs << content;
    ofs.close();
  }

  int countOccurrences(const std::string &text, const std::string &pattern) {
    int count = 0;
    size_t pos = text.find(pattern, 0);
    while (pos != std::string::npos) {
      count++;
      pos = text.find(pattern, pos + pattern.length());
    }
    return count;
  }
};

TEST_F(InterpolationOutputTest, RedundantOutputCheck) {
  writeYaml(R"(general:
  calendar_type: Standard
  time_step: 3600.0
physics:
  solver: UQ
  linear_input: LN1
  input_dissipation: ST4
  nonlinear_interactions: NL1
  bottom_friction: BT1
forcing:
  water_levels: none
  currents: none
  winds: homogeneous
  ice_concentrations: none
  bottom_depth: none
homogeneous_data:
  winds:
    - "20260101 000000 10.0 0.0"
    - "20260101 020000 15.0 0.0")");

  ww4_utils::DateTime startTime{20260101, 0.0};
  ww4_utils::DateTime endTime{20260101, 20000.0}; // 02:00:00
  std::stringstream ss;

  ww4_core::w4core_init(startTime, "test", ss);
  ww4_core::w4core_wave(startTime, endTime, ss);

  std::string output = ss.str();

  // The simulation runs from 00:00 to 02:00 with 1h step.
  // Loop iterations:
  // 1. modelTime = 00:00. Step to 01:00.
  // 2. modelTime = 01:00. Step to 02:00.
  // Loop ends.

  // Currently, it prints "Interpolation from" in both iterations.
  // We want it to print only once because the interval [00:00, 02:00] hasn't
  // changed.

  int count = countOccurrences(output, "Interpolation from");
  EXPECT_EQ(count, 1) << "Output was:\n" << output;
}
