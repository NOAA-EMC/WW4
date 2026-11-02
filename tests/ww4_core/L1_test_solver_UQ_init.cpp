#include "ww4_core/solver_UQ/utils_UQ_IO/test_ugrid_helper.h"
/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_solver_UQ_init.cpp
 * @brief Unit tests for the UQ solver initialization module.
 * @details Verifies solver_UQ_init execution.
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
 * @date Initial, 2026-10-23
 * @date Last update : 2026-10-26
 */

#include "ww4_core/solver_UQ/solver_UQ_init.h"
#include "ww4_core/w4core_init.h"
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

namespace ww4_core {

TEST(SolverUQInitTest, BasicExecutionAndOutput) {
  resetInternalState();
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "my_uq_grid.nc";
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  { ww4::test_helpers::createMinimalUgridFile("my_uq_grid.nc", 10, 10); }

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(output.find("Processing the grid:"), std::string::npos);
  EXPECT_NE(output.find("     Grid file: my_uq_grid.nc"), std::string::npos);

  std::remove("my_uq_grid.nc");
  resetInternalState();
}

} // namespace ww4_core
