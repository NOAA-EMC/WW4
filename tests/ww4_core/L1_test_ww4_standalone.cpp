/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_ww4_standalone.cpp
 * @brief Unit/integration tests for the WAVEWATCH IV stand-alone program.
 * @details This file tests the main execution of the ww4_standalone program.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 * @date Initial, 2026-07-13
 * @date Last update : 2026-07-13
 */

#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

#ifndef _WIN32
#include <sys/wait.h>
#endif

namespace ww4_core {

class WW4StandaloneL1Test : public ::testing::Test {
protected:
  void SetUp() override {
    std::remove("ww4_standalone.yaml");
    std::remove("ww4_run_config.yaml");
    std::remove("ww4_log.txt");
    std::remove("standalone_test.log");
  }

  void TearDown() override {
    std::remove("ww4_standalone.yaml");
    std::remove("ww4_run_config.yaml");
    std::remove("ww4_log.txt");
    std::remove("standalone_test.log");
  }

  int runStandalone() {
    std::string command =
        std::string(STANDALONE_EXE_PATH) + " > standalone_test.log 2>&1";
    int result = std::system(command.c_str());
#ifdef _WIN32
    return result;
#else
    return WIFEXITED(result) ? WEXITSTATUS(result) : -1;
#endif
  }
};

TEST_F(WW4StandaloneL1Test, MissingConfigFails) {
  // Executing ww4_standalone main without configuration files should fail.
  int exit_code = runStandalone();
  EXPECT_NE(exit_code, 0);
}

TEST_F(WW4StandaloneL1Test, ValidConfigSucceeds) {
  // Create a valid ww4_standalone.yaml configuration
  std::ofstream standaloneFile("ww4_standalone.yaml");
  standaloneFile << "simulation:\n"
                 << "  start_time: \"20260101 000000\"\n"
                 << "  end_time: \"20260101 020000\"\n";
  standaloneFile.close();

  // Create a valid ww4_run_config.yaml configuration
  std::ofstream runFile("ww4_run_config.yaml");
  runFile << "general:\n"
          << "  calendar_type: \"Standard\"\n"
          << "  produce_std_out: \"no\"\n"
          << "  produce_log_file: \"yes\"\n"
          << "  screen_output_level: \"none\"\n"
          << "  time_step: 3600.0\n"
          << "physics:\n"
          << "  dry_run: \"no\"\n"
          << "forcing:\n"
          << "  bottom_depth: \"from_grid\"\n"
          << "  water_levels: \"none\"\n"
          << "  currents: \"none\"\n"
          << "  winds: \"none\"\n"
          << "  ice_concentrations: \"none\"\n";
  runFile.close();

  // Executing ww4_standalone main with valid configuration should succeed.
  int exit_code = runStandalone();
  EXPECT_EQ(exit_code, 0);
}

} // namespace ww4_core
