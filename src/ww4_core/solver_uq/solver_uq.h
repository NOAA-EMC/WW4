/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file solver_uq.h
 * @brief Header for the Ultimate Quickest (UQ) regular grid solver.
 * @details Concrete implementation of the ISolver interface using UQ dynamics.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-09-15
 * @date Last update : 2026-09-25
 */

#pragma once

#include "ww4_core/solver.h"
#include "ww4_core/source_term.h"
#include <iostream>
#include <memory>
#include <span>
#include <vector>

namespace ww4_core {

// --- w4core_init_uq ---------------------------------------------------------
/**
 * @brief Initialization routine for the Ultimate Quickest (UQ) regular grid
 * solver.
 * @param os Output stream for logging.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-09-15
 * @date Last update : 2026-09-25
 */
void w4core_init_uq(std::ostream &os = std::cout);

// --- SolverRectangularGrid --------------------------------------------------
/**
 * @class SolverRectangularGrid
 * @brief Implementation of the UQ solver for regular grids.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-09-15
 * @date Last update : 2026-09-25
 */
class SolverRectangularGrid : public ISolver {
public:
  SolverRectangularGrid() = default;
  ~SolverRectangularGrid() override = default;

  [[nodiscard]] std::string_view getName() const noexcept override {
    return "UQ";
  }

  void addSourceTerm(std::unique_ptr<ISourceTerm> source) override;

  void init() override;

  void solve(std::span<double> data) override;

private:
  std::vector<std::unique_ptr<ISourceTerm>> sourceTerms_;
};

} // namespace ww4_core
