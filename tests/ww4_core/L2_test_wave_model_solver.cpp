/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L2_test_wave_model_solver.cpp
 * @brief Integration tests for the WaveModelSolver class with integrated
 * physics.
 * @details Verifies the integration of solvers and source term physics in the
 *          orchestration loop of the main WaveModelSolver.
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
 * @date Initial, 2026-06-24
 * @date Last update : 2026-09-28
 */

#include "ww4_core/scheme_factory.h"
#include "ww4_core/w4core_init.h"
#include "ww4_core/wave_model_solver.h"
#include <gtest/gtest.h>
#include <vector>

namespace ww4_core {

TEST(WaveModelSolverTest, SimulationStep) {
  resetInternalState();
  getMutableRunConfig().linearInput = ww4_utils::LinearInputScheme::LN1;
  getMutableRunConfig().inputDissipation =
      ww4_utils::InputDissipationScheme::ST4;
  getMutableRunConfig().nonlinearInteractions = ww4_utils::NonlinearScheme::NL1;
  getMutableRunConfig().bottomFriction = ww4_utils::BottomFrictionScheme::BT1;

  WaveModelSolver model;

  // Assemble the model: SolverUQ solver with ComputeAllSources source
  // term
  auto solver = SchemeFactory::createSolver("SolverUQ");
  auto source = SchemeFactory::createSourceTerm("compute_all_sources");
  solver->addSourceTerm(std::move(source));

  model.initialize(std::move(solver));

  std::vector<double> initialData = {1.0, 2.0, 3.0};
  model.setData(initialData);

  model.step();

  auto result = model.getData();
  ASSERT_EQ(result.size(), initialData.size());

  // UQ multiplies by 1.01, ComputeAllSources applies LN1 (+0.005), ST4 (+0.04),
  // NL1 (+0.001), BT1 (-0.001) -> +0.045
  for (size_t i = 0; i < initialData.size(); ++i) {
    double expected = initialData[i] * 1.01 + 0.045;
    EXPECT_NEAR(result[i], expected, 1e-9);
  }

  resetInternalState();
}

} // namespace ww4_core
