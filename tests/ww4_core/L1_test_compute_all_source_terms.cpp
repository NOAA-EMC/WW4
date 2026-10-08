/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_compute_all_source_terms.cpp
 * @brief Unit tests for physical source term calculations (ComputeAllSources).
 * @details Verifies the behavior and interface compliance of ComputeAllSources.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 *
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 *
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-09-15
 * @date Last update : 2026-09-29
 */

#include "ww4_core/source_terms/compute_all_source_terms.h"
#include "ww4_core/w4core_init.h"
#include <fstream>
#include <gtest/gtest.h>
#include <vector>

namespace ww4_core {

class ComputeAllSourcesL1Test : public ::testing::Test {
protected:
  void SetUp() override { resetInternalState(); }

  void TearDown() override {
    resetInternalState();
    std::remove("ww4_run_config.yaml");
    std::remove("ww4_log.txt");
  }

  void writeYaml(const std::string &inputDiss, const std::string &nlInter,
                 const std::string &linInput = "none",
                 const std::string &botFric = "none", bool sourceTerms = true) {
    std::ofstream runFile("ww4_run_config.yaml");
    runFile << "general:\n";
    runFile << "  time_step: 3600.0\n";
    runFile << "physics:\n";
    runFile << "  solver: UQ\n";
    runFile << "  source_terms: " << (sourceTerms ? "yes" : "no") << "\n";
    runFile << "  linear_input: " << linInput << "\n";
    runFile << "  input_dissipation: " << inputDiss << "\n";
    runFile << "  nonlinear_interactions: " << nlInter << "\n";
    runFile << "  bottom_friction: " << botFric << "\n";
    runFile << "forcing:\n";
    runFile << "  water_levels: none\n";
    runFile << "  currents: none\n";
    runFile << "  winds: none\n";
    runFile << "  ice_concentrations: none\n";
    runFile.close();
  }
};

TEST_F(ComputeAllSourcesL1Test, NameCheck) {
  ComputeAllSources sources;
  EXPECT_EQ(sources.getName(), "ComputeAllSources");
}

TEST_F(ComputeAllSourcesL1Test, CalculateST1AndNL1) {
  writeYaml("ST1", "NL1");
  ww4_utils::DateTime startTime = {20260101, 0.0};
  w4core_init(startTime, "test_sources", std::cout);

  ComputeAllSources sources;
  sources.init();

  std::vector<double> data = {1.0, 2.5};
  sources.calculate(data);

  // ST1 adds 0.01, NL1 adds 0.001 -> total + 0.011
  EXPECT_NEAR(data[0], 1.011, 1e-9);
  EXPECT_NEAR(data[1], 2.511, 1e-9);
}

TEST_F(ComputeAllSourcesL1Test, CalculateST4AndNL3) {
  writeYaml("ST4", "NL3");
  ww4_utils::DateTime startTime = {20260101, 0.0};
  w4core_init(startTime, "test_sources", std::cout);

  ComputeAllSources sources;
  std::vector<double> data = {1.0, 2.5};
  sources.calculate(data); // Implicit init

  // ST4 adds 0.04, NL3 adds 0.003 -> total + 0.043
  EXPECT_NEAR(data[0], 1.043, 1e-9);
  EXPECT_NEAR(data[1], 2.543, 1e-9);
}

TEST_F(ComputeAllSourcesL1Test, CalculateST2ST6AndNL2) {
  writeYaml("ST2", "NL2");
  ww4_utils::DateTime startTime = {20260101, 0.0};
  w4core_init(startTime, "test_sources", std::cout);

  ComputeAllSources sources;
  std::vector<double> data = {1.0, 2.5};
  sources.calculate(data);

  // ST2 adds 0.02, NL2 adds 0.002 -> total + 0.022
  EXPECT_NEAR(data[0], 1.022, 1e-9);
  EXPECT_NEAR(data[1], 2.522, 1e-9);

  resetInternalState();
  writeYaml("ST6", "NL2");
  w4core_init(startTime, "test_sources_st6", std::cout);

  ComputeAllSources sources2;
  data = {1.0, 2.5};
  sources2.calculate(data);

  // ST6 adds 0.06, NL2 adds 0.002 -> total + 0.062
  EXPECT_NEAR(data[0], 1.062, 1e-9);
  EXPECT_NEAR(data[1], 2.562, 1e-9);
}

TEST_F(ComputeAllSourcesL1Test, CalculateLN1AndBT1BT4) {
  writeYaml("none", "none", "LN1", "BT1");
  ww4_utils::DateTime startTime = {20260101, 0.0};
  w4core_init(startTime, "test_sources_ln1_bt1", std::cout);

  ComputeAllSources sources;
  std::vector<double> data = {1.0, 2.5};
  sources.calculate(data);

  // LN1 adds 0.005, BT1 subtracts 0.001 -> total + 0.004
  EXPECT_NEAR(data[0], 1.004, 1e-9);
  EXPECT_NEAR(data[1], 2.504, 1e-9);

  resetInternalState();
  writeYaml("none", "none", "none", "BT4");
  w4core_init(startTime, "test_sources_bt4", std::cout);

  ComputeAllSources sources2;
  data = {1.0, 2.5};
  sources2.calculate(data);

  // BT4 subtracts 0.004
  EXPECT_NEAR(data[0], 0.996, 1e-9);
  EXPECT_NEAR(data[1], 2.496, 1e-9);
}

TEST_F(ComputeAllSourcesL1Test, CalculateDoNotUse) {
  writeYaml("none", "none");
  ww4_utils::DateTime startTime = {20260101, 0.0};
  w4core_init(startTime, "test_sources", std::cout);

  ComputeAllSources sources;
  sources.init();

  std::vector<double> data = {1.0, 2.5};
  sources.calculate(data);

  // No source term modifications
  EXPECT_DOUBLE_EQ(data[0], 1.0);
  EXPECT_DOUBLE_EQ(data[1], 2.5);
}

TEST_F(ComputeAllSourcesL1Test, DisabledSourceTerms) {
  writeYaml("ST4", "NL3", "none", "none",
            false); // source_terms: no
  ww4_utils::DateTime startTime = {20260101, 0.0};
  w4core_init(startTime, "test_sources", std::cout);

  ComputeAllSources sources;
  std::vector<double> data = {1.0, 2.5};
  sources.calculate(data);

  // Disabled source terms -> no change
  EXPECT_DOUBLE_EQ(data[0], 1.0);
  EXPECT_DOUBLE_EQ(data[1], 2.5);
}

} // namespace ww4_core
