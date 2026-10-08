/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file scheme_factory.cpp
 * @brief Implementation of the scheme factory.
 * @details Handles the instantiation and assembly of model components.
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

#include "ww4_core/scheme_factory.h"

#include "ww4_core/solver_smc/solver_smc.h"
#include "ww4_core/solver_triangular/solver_triangular.h"
#include "ww4_core/solver_uq/solver_uq.h"

#include "ww4_core/source_terms/compute_all_source_terms.h"

#include "ww4_core/source_terms/ww4_LN1/ww4_LN1.h"

#include "ww4_core/source_terms/ww4_ST1/ww4_ST1.h"
#include "ww4_core/source_terms/ww4_ST2/ww4_ST2.h"
#include "ww4_core/source_terms/ww4_ST4/ww4_ST4.h"
#include "ww4_core/source_terms/ww4_ST6/ww4_ST6.h"

#include "ww4_core/source_terms/ww4_NL1/ww4_NL1.h"
#include "ww4_core/source_terms/ww4_NL2/ww4_NL2.h"
#include "ww4_core/source_terms/ww4_NL3/ww4_NL3.h"

#include "ww4_core/source_terms/ww4_BT1/ww4_BT1.h"
#include "ww4_core/source_terms/ww4_BT4/ww4_BT4.h"
#include <stdexcept>

namespace ww4_core {

// --- createSolver -----------------------------------------------------------
/**
 * @brief Instantiates a solver scheme by string name.
 * @param name Name of requested solver.
 * @return Unique pointer to created solver.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-06-24
 * @date Last update : 2026-09-29
 */
std::unique_ptr<ISolver> SchemeFactory::createSolver(const std::string &name) {
  if (name == "SolverUQ") {
    return std::make_unique<SolverRectangularGrid>();
  }
  if (name == "SolverTriangular") {
    return std::make_unique<SolverTriangularGrid>();
  }
  if (name == "SolverSMC") {
    return std::make_unique<SolverSMCGrid>();
  }
  throw std::invalid_argument("Unknown solver: " + name);
}

// --- createSourceTerm -------------------------------------------------------
/**
 * @brief Instantiates a source term scheme by string name.
 * @param name Name of requested source term.
 * @return Unique pointer to created source term.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-06-24
 * @date Last update : 2026-09-29
 */
std::unique_ptr<ISourceTerm>
SchemeFactory::createSourceTerm(const std::string &name) {
  if (name == "ComputeAllSources" || name == "compute_all_sources") {
    return std::make_unique<ComputeAllSources>();
  }
  if (name == "LN1" || name == "ln1") {
    return std::make_unique<SourceTermLN1>();
  }
  if (name == "ST1" || name == "st1") {
    return std::make_unique<SourceTermST1>();
  }
  if (name == "ST2" || name == "st2") {
    return std::make_unique<SourceTermST2>();
  }
  if (name == "ST4" || name == "st4") {
    return std::make_unique<SourceTermST4>();
  }
  if (name == "ST6" || name == "st6") {
    return std::make_unique<SourceTermST6>();
  }
  if (name == "NL1" || name == "nl1") {
    return std::make_unique<SourceTermNL1>();
  }
  if (name == "NL2" || name == "nl2") {
    return std::make_unique<SourceTermNL2>();
  }
  if (name == "NL3" || name == "nl3") {
    return std::make_unique<SourceTermNL3>();
  }
  if (name == "BT1" || name == "bt1") {
    return std::make_unique<SourceTermBT1>();
  }
  if (name == "BT4" || name == "bt4") {
    return std::make_unique<SourceTermBT4>();
  }
  throw std::invalid_argument("Unknown source term scheme: " + name);
}

} // namespace ww4_core
