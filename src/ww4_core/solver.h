/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file solver.h
 * @brief Interface for numerical solvers in WAVEWATCH IV.
 * @details Defines the abstract base class for model solvers that integrate
 *          dynamics and physics.
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

#include "ww4_core/source_term.h"
#include <memory>
#include <span>
#include <string_view>

namespace ww4_core {

// --- ISolver ----------------------------------------------------------------
/**
 * @class ISolver
 * @brief Abstract base class for wave model solvers.
 * @details A solver is responsible for advancing the model state in time,
 *          integrating both propagation and source terms.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-06-24
 * @date Last update : 2026-09-25
 */
class ISolver {
public:
  virtual ~ISolver() = default;

  // --- Get the name of the solver -------------------------------------------
  /**
   * @brief Get the name of the solver.
   * @return String view containing the solver name.
   */
  [[nodiscard]] virtual std::string_view getName() const noexcept = 0;

  // --- Add a source term to be handled by this solver -----------------------
  /**
   * @brief Add a source term to be handled by this solver.
   * @param source Unique pointer to the source term scheme.
   */
  virtual void addSourceTerm(std::unique_ptr<ISourceTerm> source) = 0;

  // --- Initialize solver state and data structures --------------------------
  /**
   * @brief Initialize solver state and data structures.
   */
  virtual void init() {}

  // --- Solve the wave action balance equation for one step ------------------
  /**
   * @brief Solve the wave action balance equation for one step.
   * @param data Model data to be updated.
   */
  virtual void solve(std::span<double> data) = 0;
};

} // namespace ww4_core
