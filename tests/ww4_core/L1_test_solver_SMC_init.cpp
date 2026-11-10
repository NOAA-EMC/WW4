/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_solver_SMC_init.cpp
 * @brief Unit tests for the SMC grid solver initialization module.
 * @details Verifies solver_SMC_init execution.
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

#include "ww4_core/solver_SMC/solver_SMC_init.h"
#include <gtest/gtest.h>
#include <sstream>

namespace ww4_core {

TEST(SolverSMCInitTest, AbortsOnBottomGrid) {
  EXPECT_EXIT(
      solver_SMC_init(std::cerr), ::testing::ExitedWithCode(1),
      "WW4 ERROR: SMC grid reader not yet implemented when reading bottom "
      "grid.");
}

} // namespace ww4_core
