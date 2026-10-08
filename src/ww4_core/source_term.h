/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file source_term.h
 * @brief Interface for source term calculations in WAVEWATCH IV.
 * @details Defines the abstract base class for different physical source terms.
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

#include <span>
#include <string_view>

namespace ww4_core {

// --- ISourceTerm ------------------------------------------------------------
/**
 * @class ISourceTerm
 * @brief Abstract base class for physical source terms in WAVEWATCH IV.
 * @details Interface providing physical source term calculations.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-06-24
 * @date Last update : 2026-09-25
 */
class ISourceTerm {
public:
  virtual ~ISourceTerm() = default;

  // --- Get name of scheme ----------------------------------------------------
  /**
   * @brief Get the name of the source term scheme.
   * @return String view containing scheme name.
   */
  [[nodiscard]] virtual std::string_view getName() const noexcept = 0;

  // --- Calculate changes -----------------------------------------------------
  /**
   * @brief Calculate source term changes for given model data.
   * @param data Span of model data to update.
   */
  virtual void calculate(std::span<double> data) = 0;
};

} // namespace ww4_core
