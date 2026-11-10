/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_solver_SMC_exec.cpp
 * @brief Unit tests for the SMC grid solver module.
 * @details Verifies initialization, adding source terms, and solver execution
 * for SolverSMCGrid.
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
 * @date Initial, 2026-10-15
 * @date Last update : 2026-10-26
 */

#include "ww4_core/scheme_factory.h"
#include "ww4_core/solver_SMC/solver_SMC_exec.h"
#include "ww4_core/solver_SMC/solver_SMC_init.h"
#include <gtest/gtest.h>

namespace ww4_core {

TEST(SolverSMCTest, BasicPropertiesAndInit) {
  SolverSMCGrid solver;
  EXPECT_EQ(solver.getName(), "SMC");

  EXPECT_EXIT(
      solver.init(), ::testing::ExitedWithCode(1),
      "WW4 ERROR: SMC grid reader not yet implemented when reading bottom "
      "grid.");
}

TEST(SolverSMCTest, SolveAndAddSourceTerm) {
  SolverSMCGrid solver;
  std::vector<double> data = {1.0, 2.0, 3.0};

  // Execution without prior addSourceTerm triggers auto-creation of
  // ComputeAllSources
  solver.solve(data);
  EXPECT_GT(data[0], 1.0);

  // Explicitly add source term and solve again
  auto source = SchemeFactory::createSourceTerm("ST1");
  solver.addSourceTerm(std::move(source));
  EXPECT_NO_THROW(solver.solve(data));
}

} // namespace ww4_core
