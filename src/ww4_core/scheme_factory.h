/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file scheme_factory.h
 * @brief Factory for creating solvers and source term schemes at runtime.
 * @details Implements a registry-based factory for model components.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-06-24
 * @date Last update : 2026-09-29
 */

#pragma once

#include "ww4_core/solver.h"
#include "ww4_core/source_term.h"
#include <memory>
#include <string>

namespace ww4_core {

// --- SchemeFactory ----------------------------------------------------------
/**
 * @class SchemeFactory
 * @brief Factory class for creating model solvers and source terms.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-06-24
 * @date Last update : 2026-09-28
 */
class SchemeFactory {
public:
  /**
   * @brief Create a solver scheme.
   * @param name The name of the solver (e.g., "SolverUQ", "SolverTriangular",
   * "SolverSMC").
   * @return A unique pointer to the created ISolver.
   */
  static std::unique_ptr<ISolver> createSolver(const std::string &name);

  /**
   * @brief Create a source term scheme.
   * @param name The name of the scheme (e.g., "ST4").
   * @return A unique pointer to the created ISourceTerm.
   */
  static std::unique_ptr<ISourceTerm> createSourceTerm(const std::string &name);
};

} // namespace ww4_core
