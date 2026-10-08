/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L2_test_w4core_input_cycling.cpp
 * @brief Unit tests for homogeneous input cycling logic.
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

#include "ww4_core/w4core_init.h"
#include "ww4_core/w4core_wave.h"
#include "ww4_utils/time_management.h"
#include "ww4_utils/ww4_input_utils.h"
#include <fstream>
#include <gtest/gtest.h>

namespace {

class W4CoreInputCyclingTest : public ::testing::Test {
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
};

TEST_F(W4CoreInputCyclingTest, CyclingBeforeData) {
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
    - "20260101 120000 10.0 0.0"
    - "20260101 180000 15.0 0.0")");

  ww4_utils::DateTime startTime{20260101, 0.0};
  ww4_utils::DateTime endTime{20260101, 230000.0};
  std::stringstream ss;
  ww4_core::w4core_init(startTime, "test", ss);

  ww4_utils::intTimeData data;
  ww4_utils::ww4_hom_winds(startTime, endTime, data);

  ASSERT_TRUE(data.time1.has_value());
  ASSERT_TRUE(data.time2.has_value());
  EXPECT_EQ(data.time1->ymd, 20260101);
  EXPECT_NEAR(data.time1->hms, 0.0, 1e-6);
  EXPECT_EQ(data.time2->ymd, 20260101);
  EXPECT_NEAR(data.time2->hms, 120000.0, 1e-6);
  EXPECT_NEAR(data.maxStep, 12.0 * 3600.0, 1e-6);
}

TEST_F(W4CoreInputCyclingTest, CyclingDuringData) {
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
    - "20260101 120000 15.0 0.0"
    - "20260101 180000 20.0 0.0")");

  ww4_utils::DateTime startTime{20260101, 60000.0}; // 06:00
  ww4_utils::DateTime endTime{20260101, 230000.0};
  std::stringstream ss;
  ww4_core::w4core_init(startTime, "test", ss);

  ww4_utils::intTimeData data;
  ww4_utils::ww4_hom_winds(startTime, endTime, data);

  ASSERT_TRUE(data.time1.has_value());
  ASSERT_TRUE(data.time2.has_value());
  EXPECT_EQ(data.time1->ymd, 20260101);
  EXPECT_NEAR(data.time1->hms, 0.0, 1e-6);
  EXPECT_EQ(data.time2->ymd, 20260101);
  EXPECT_NEAR(data.time2->hms, 120000.0, 1e-6);
  EXPECT_NEAR(data.maxStep, 6.0 * 3600.0, 1e-6);
}

TEST_F(W4CoreInputCyclingTest, CyclingAfterData) {
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
    - "20260101 120000 15.0 0.0")");

  ww4_utils::DateTime startTime{20260101, 180000.0}; // 18:00
  ww4_utils::DateTime endTime{20260101, 230000.0};
  std::stringstream ss;
  ww4_core::w4core_init(startTime, "test", ss);

  ww4_utils::intTimeData data;
  ww4_utils::ww4_hom_winds(startTime, endTime, data);

  ASSERT_TRUE(data.time1.has_value());
  ASSERT_TRUE(data.time2.has_value());
  EXPECT_EQ(data.time1->ymd, 20260101);
  EXPECT_NEAR(data.time1->hms, 120000.0, 1e-6);
  EXPECT_EQ(data.time2->ymd, 20260101);
  EXPECT_NEAR(data.time2->hms, 230000.0, 1e-6);
  EXPECT_NEAR(data.maxStep, 5.0 * 3600.0, 1e-6);
}

TEST_F(W4CoreInputCyclingTest, CyclingEmptyData) {
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
  bottom_depth: none)");

  ww4_utils::DateTime startTime{20260101, 0.0};
  EXPECT_EXIT(ww4_core::w4core_init(startTime, "test", std::cerr),
              ::testing::ExitedWithCode(1),
              "No data provided for homogeneous field");
}

TEST_F(W4CoreInputCyclingTest, IntegrationWithWaveLoop) {
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
    - "20260101 003000 15.0 0.0")");

  ww4_utils::DateTime startTime{20260101, 0.0};
  ww4_utils::DateTime endTime{20260101, 10000.0}; // 01:00
  std::stringstream ss;
  ww4_core::w4core_init(startTime, "test", ss);

  // Run wave loop. First step should be 1800s (to reach 00:30:00) instead of
  // 3600s.
  ww4_core::w4core_wave(startTime, endTime, ss);

  // We can't easily check internal steps, but we can check the final model
  // time.
  EXPECT_EQ(ww4_core::getWaveTimeData().modelTime->ymd, 20260101);
  EXPECT_NEAR(ww4_core::getWaveTimeData().modelTime->hms, 10000.0, 1e-6);

  std::string output = ss.str();
  EXPECT_NE(output.find("Updating winds"), std::string::npos);
  EXPECT_NE(output.find("Interpolation from 2026/01/01 00:00:00 UTC to "
                        "2026/01/01 00:30:00 UTC"),
            std::string::npos);
}

} // namespace
