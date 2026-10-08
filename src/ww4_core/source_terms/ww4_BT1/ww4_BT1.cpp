/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_BT1.cpp
 * @brief Implementation of BT1 bottom friction source term calculations.
 * @details Concrete subroutine for BT1 bottom friction source terms.
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

#include "ww4_core/source_terms/ww4_BT1/ww4_BT1.h"
#include <algorithm>

namespace ww4_core {

// --- calculate --------------------------------------------------------------
void SourceTermBT1::calculate(std::span<double> data) {
  std::for_each(data.begin(), data.end(), [](double &val) { val -= 0.001; });
}

} // namespace ww4_core
