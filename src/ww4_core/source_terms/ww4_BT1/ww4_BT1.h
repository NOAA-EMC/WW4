/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_BT1.h
 * @brief Header for BT1 bottom friction source term calculations.
 * @details Concrete implementation of ISourceTerm for BT1 scheme.
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

// --- SourceTermBT1 ----------------------------------------------------------
/**
 * @class SourceTermBT1
 * @brief Implementation of BT1 bottom friction source terms.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-09-24
 * @date Last update : 2026-09-29
 */
class SourceTermBT1 : public ISourceTerm {
public:
  SourceTermBT1() = default;
  ~SourceTermBT1() override = default;

  [[nodiscard]] std::string_view getName() const noexcept override {
    return "BT1";
  }

  void calculate(std::span<double> data) override;
};

} // namespace ww4_core
