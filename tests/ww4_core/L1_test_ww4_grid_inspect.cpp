/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_ww4_grid_inspect.cpp
 * @brief Unit/integration tests for the WW4 NetCDF grid inspection tool.
 * @details Tests the main execution of the ww4_grid_inspect program with valid
 *          and invalid grid files.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-11-04
 * @date Last update : 2026-11-04
 */

#include "ww4_core/solver_UQ/utils_UQ_IO/test_ugrid_helper.h"
#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

#ifndef _WIN32
#include <sys/wait.h>
#endif

namespace ww4_core {

// --- WW4GridInspectL1Test ---------------------------------------------------
/**
 * @class WW4GridInspectL1Test
 * @brief Test fixture for testing ww4_grid_inspect executable.
 */
class WW4GridInspectL1Test : public ::testing::Test {
protected:
  void SetUp() override {
    std::remove("test_inspect_grid.nc");
    std::remove("inspect_test.log");
    ww4::test_helpers::createMinimalUgridFile("test_inspect_grid.nc", 10, 10);
  }

  void TearDown() override {
    std::remove("test_inspect_grid.nc");
    std::remove("corrupt_inspect_grid.nc");
    std::remove("inspect_test.log");
  }

  int runInspect(const std::string &args = "") {
    std::string command = std::string(GRID_INSPECT_EXE_PATH) +
                          (args.empty() ? "" : " " + args) +
                          " > inspect_test.log 2>&1";
    int result = std::system(command.c_str());
#ifdef _WIN32
    return result;
#else
    return WIFEXITED(result) ? WEXITSTATUS(result) : -1;
#endif
  }

  std::string readLog() {
    std::ifstream ifs("inspect_test.log");
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       std::istreambuf_iterator<char>());
  }
};

// --- MissingArgsFails -------------------------------------------------------
TEST_F(WW4GridInspectL1Test, MissingArgsFails) {
  int exit_code = runInspect();
  EXPECT_NE(exit_code, 0);

  std::string log = readLog();
  EXPECT_NE(log.find("*** WAVEWATCH IV program ww4_grid_inspect ***"),
            std::string::npos);
  EXPECT_NE(log.find("Usage: ww4_grid_inspect <grid_file.nc>"),
            std::string::npos);
  EXPECT_NE(log.find("Missing grid file command line argument"),
            std::string::npos);
}

// --- NonExistentGridFileFails -----------------------------------------------
TEST_F(WW4GridInspectL1Test, NonExistentGridFileFails) {
  int exit_code = runInspect("non_existent_grid_file.nc");
  EXPECT_NE(exit_code, 0);

  std::string log = readLog();
  EXPECT_NE(log.find("*** WAVEWATCH IV program ww4_grid_inspect ***"),
            std::string::npos);
  EXPECT_NE(log.find("Processing the grid:"), std::string::npos);
  EXPECT_NE(log.find("Grid file                : non_existent_grid_file.nc"),
            std::string::npos);
  EXPECT_NE(log.find("Grid file does not exist or failed opening"),
            std::string::npos);
}

// --- CorruptedGridFileFails -------------------------------------------------
TEST_F(WW4GridInspectL1Test, CorruptedGridFileFails) {
  {
    std::ofstream ofs("corrupt_inspect_grid.nc");
    ofs << "This is not a valid NetCDF file content.";
  }

  int exit_code = runInspect("corrupt_inspect_grid.nc");
  EXPECT_NE(exit_code, 0);

  std::string log = readLog();
  EXPECT_NE(log.find("*** WAVEWATCH IV program ww4_grid_inspect ***"),
            std::string::npos);
  EXPECT_NE(log.find("Processing the grid:"), std::string::npos);
  EXPECT_NE(log.find("Grid file                : corrupt_inspect_grid.nc"),
            std::string::npos);
  EXPECT_NE(log.find("Grid file does not exist or failed opening"),
            std::string::npos);
}

// --- ValidGridFileSucceeds --------------------------------------------------
TEST_F(WW4GridInspectL1Test, ValidGridFileSucceeds) {
  int exit_code = runInspect("test_inspect_grid.nc");
  EXPECT_EQ(exit_code, 0);

  std::string log = readLog();
  EXPECT_NE(log.find("*** WAVEWATCH IV program ww4_grid_inspect ***"),
            std::string::npos);
  EXPECT_NE(log.find("Processing the grid:"), std::string::npos);
  EXPECT_NE(log.find("Grid file                : test_inspect_grid.nc"),
            std::string::npos);
  EXPECT_NE(log.find("Grid metadata:"), std::string::npos);
  EXPECT_NE(log.find("Grid dimensions       : NX = 10, NY = 10"),
            std::string::npos);
  EXPECT_NE(log.find("Number of faces       : 100"), std::string::npos);
  EXPECT_NE(log.find("End of program"), std::string::npos);
  EXPECT_NE(log.find("WAVEWATCH IV program ww4_grid_inspect"),
            std::string::npos);
}

} // namespace ww4_core
