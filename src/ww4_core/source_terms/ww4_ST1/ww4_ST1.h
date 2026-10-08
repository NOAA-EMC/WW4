/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_ST1.h
 * @brief Header for ST1 input and dissipation source term calculations.
 * @details Concrete implementation of ISourceTerm for ST1 scheme.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-09-24
 * @date Last update : 2026-09-29
 */

#pragma once

#include "ww4_core/source_term.h"
#include <span>

namespace ww4_core {

// --- SourceTermST1 ----------------------------------------------------------
/**
 * @class SourceTermST1
 * @brief Implementation of ST1 input and dissipation source terms.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-09-24
 * @date Last update : 2026-09-29
 */
class SourceTermST1 : public ISourceTerm {
public:
  SourceTermST1() = default;
  ~SourceTermST1() override = default;

  [[nodiscard]] std::string_view getName() const noexcept override {
    return "ST1";
  }

  void calculate(std::span<double> data) override;
};

} // namespace ww4_core
