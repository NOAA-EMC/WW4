/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file wave_model_solver.cpp
 * @brief Implementation of the WaveModelSolver class.
 * @details Orchestrates the model initialization and execution loop.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-06-24
 * @date Last update : 2026-09-25
 */

#include "ww4_core/wave_model_solver.h"

namespace ww4_core {

// --- initialize -------------------------------------------------------------
void WaveModelSolver::initialize(std::unique_ptr<ISolver> solver) {
  solver_ = std::move(solver);
}

// --- setData ----------------------------------------------------------------
void WaveModelSolver::setData(std::vector<double> initialData) {
  data_ = std::move(initialData);
}

// --- step -------------------------------------------------------------------
void WaveModelSolver::step() {
  if (solver_) {
    solver_->solve(data_);
  }
}

} // namespace ww4_core
