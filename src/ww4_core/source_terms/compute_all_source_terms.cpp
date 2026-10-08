/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file compute_all_source_terms.cpp
 * @brief Implementation of physical source term calculations.
 * @details Concrete subroutine for physical source terms in WW4 core.
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

#include "ww4_core/source_terms/compute_all_source_terms.h"
#include "ww4_core/scheme_factory.h"
#include "ww4_core/w4core_init.h"
#include "ww4_utils/ww4_run_config.h"

namespace ww4_core {

// --- init -------------------------------------------------------------------
void ComputeAllSources::init() {
  const auto &config = getRunConfig();

  // === Instantiate Linear Input Scheme =====================================
  switch (config.linearInput) {
  case ww4_utils::LinearInputScheme::LN1:
    linearInputTerm_ = SchemeFactory::createSourceTerm("LN1");
    break;
  default:
    linearInputTerm_ = nullptr;
    break;
  }

  // === Instantiate Input and Dissipation Scheme ============================
  switch (config.inputDissipation) {
  case ww4_utils::InputDissipationScheme::ST1:
    inputDissipationTerm_ = SchemeFactory::createSourceTerm("ST1");
    break;
  case ww4_utils::InputDissipationScheme::ST2:
    inputDissipationTerm_ = SchemeFactory::createSourceTerm("ST2");
    break;
  case ww4_utils::InputDissipationScheme::ST4:
    inputDissipationTerm_ = SchemeFactory::createSourceTerm("ST4");
    break;
  case ww4_utils::InputDissipationScheme::ST6:
    inputDissipationTerm_ = SchemeFactory::createSourceTerm("ST6");
    break;
  default:
    inputDissipationTerm_ = nullptr;
    break;
  }

  // === Instantiate Nonlinear Interactions Scheme ===========================
  switch (config.nonlinearInteractions) {
  case ww4_utils::NonlinearScheme::NL1:
    nonlinearTerm_ = SchemeFactory::createSourceTerm("NL1");
    break;
  case ww4_utils::NonlinearScheme::NL2:
    nonlinearTerm_ = SchemeFactory::createSourceTerm("NL2");
    break;
  case ww4_utils::NonlinearScheme::NL3:
    nonlinearTerm_ = SchemeFactory::createSourceTerm("NL3");
    break;
  default:
    nonlinearTerm_ = nullptr;
    break;
  }

  // === Instantiate Bottom Friction Scheme ==================================
  switch (config.bottomFriction) {
  case ww4_utils::BottomFrictionScheme::BT1:
    bottomFrictionTerm_ = SchemeFactory::createSourceTerm("BT1");
    break;
  case ww4_utils::BottomFrictionScheme::BT4:
    bottomFrictionTerm_ = SchemeFactory::createSourceTerm("BT4");
    break;
  default:
    bottomFrictionTerm_ = nullptr;
    break;
  }

  initialized_ = true;
}

// --- calculate --------------------------------------------------------------
void ComputeAllSources::calculate(std::span<double> data) {
  if (!getRunConfig().sourceTerms) {
    return;
  }

  if (!initialized_) {
    init();
  }

  if (linearInputTerm_) {
    linearInputTerm_->calculate(data);
  }

  if (inputDissipationTerm_) {
    inputDissipationTerm_->calculate(data);
  }

  if (nonlinearTerm_) {
    nonlinearTerm_->calculate(data);
  }

  if (bottomFrictionTerm_) {
    bottomFrictionTerm_->calculate(data);
  }
}

} // namespace ww4_core
