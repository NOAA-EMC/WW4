/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file compute_all_source_terms.h
 * @brief Header for physical source term calculations in WW4 core.
 * @details Concrete implementation of ISourceTerm interface for computing all
 * source terms.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-09-15
 * @date Last update : 2026-09-29
 */

#pragma once

#include "ww4_core/source_term.h"
#include <memory>
#include <span>

namespace ww4_core {

// --- ComputeAllSources ------------------------------------------------------
/**
 * @class ComputeAllSources
 * @brief Implementation for physical source term calculations.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-09-15
 * @date Last update : 2026-09-29
 */
class ComputeAllSources : public ISourceTerm {
public:
  ComputeAllSources() = default;
  ~ComputeAllSources() override = default;

  [[nodiscard]] std::string_view getName() const noexcept override {
    return "ComputeAllSources";
  }

  void init();

  void calculate(std::span<double> data) override;

private:
  std::unique_ptr<ISourceTerm> linearInputTerm_{nullptr};
  std::unique_ptr<ISourceTerm> inputDissipationTerm_{nullptr};
  std::unique_ptr<ISourceTerm> nonlinearTerm_{nullptr};
  std::unique_ptr<ISourceTerm> bottomFrictionTerm_{nullptr};
  bool initialized_{false};
};

} // namespace ww4_core
