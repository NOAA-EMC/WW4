/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file solver_triangular_exec.h
 * @brief Header for the triangular unstructured grid solver.
 * @details Concrete implementation of the ISolver interface using triangular
 * grid approach.
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

#pragma once

#include "ww4_core/solver.h"
#include "ww4_core/solver_triangular/solver_triangular_init.h"
#include "ww4_core/source_term.h"
#include <iostream>
#include <memory>
#include <span>
#include <vector>

namespace ww4_core {

// --- SolverTriangularGrid ---------------------------------------------------
/**
 * @class SolverTriangularGrid
 * @brief Implementation of the triangular unstructured grid solver.
 * @details Implements the ISolver lifecycle methods (init, solve,
 * addSourceTerm) for triangular unstructured grids.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-09-15
 * @date Last update : 2026-11-09
 */
class SolverTriangularGrid : public ISolver {
public:
  SolverTriangularGrid() = default;
  ~SolverTriangularGrid() override = default;

  /**
   * @brief Retrieves the solver scheme name.
   * @return String view representing the solver name ("Triangular").
   */
  [[nodiscard]] std::string_view getName() const noexcept override {
    return "Triangular";
  }

  /**
   * @brief Registers an external physical source term component.
   * @param[in] source Unique pointer to an ISourceTerm implementation.
   */
  void addSourceTerm(std::unique_ptr<ISourceTerm> source) override;

  /**
   * @brief Initializes the solver using solver_triangular_init.
   */
  void init() override;

  /**
   * @brief Executes wave action propagation and evaluates source terms.
   * @param[in,out] data Span of wave action spectrum data values.
   */
  void solve(std::span<double> data) override;

private:
  /// Registered physical source terms evaluated during execution
  std::vector<std::unique_ptr<ISourceTerm>> sourceTerms_;
};

} // namespace ww4_core
