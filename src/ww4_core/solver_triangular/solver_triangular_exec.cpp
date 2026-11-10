/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file solver_triangular_exec.cpp
 * @brief Implementation of the triangular grid solver execution.
 * @details Concrete implementation of the ISolver interface for triangular
 * unstructured grids.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-09-15
 * @date Last update : 2026-11-09
 */

#include "ww4_core/solver_triangular/solver_triangular_exec.h"
#include "ww4_core/scheme_factory.h"
#include "ww4_core/solver_triangular/solver_triangular_init.h"
#include "ww4_core/w4core_init.h"
#include <algorithm>

namespace ww4_core {

// --- init -------------------------------------------------------------------
void SolverTriangularGrid::init() { solver_triangular_init(); }

// --- addSourceTerm ----------------------------------------------------------
void SolverTriangularGrid::addSourceTerm(std::unique_ptr<ISourceTerm> source) {
  if (source) {
    sourceTerms_.push_back(std::move(source));
  }
}

// --- solve ------------------------------------------------------------------
void SolverTriangularGrid::solve(std::span<double> data) {
  // === Perform numerical propagation =======================================
  std::for_each(data.begin(), data.end(), [](double &val) {
    val *= 1.02; // Mock propagation effect
  });

  // === Call integrated source terms ==========================================
  if (sourceTerms_.empty()) {
    addSourceTerm(SchemeFactory::createSourceTerm("ComputeAllSources"));
  }
  for (auto &source : sourceTerms_) {
    source->calculate(data);
  }
}

} // namespace ww4_core
