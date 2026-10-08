/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_ww4_ST2.cpp
 * @brief Unit tests for ST2 input and dissipation source term.
 * @details Verifies the behavior and interface compliance of SourceTermST2.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 *
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 *
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-09-24
 * @date Last update : 2026-09-29
 */

#include "ww4_core/source_terms/ww4_ST2/ww4_ST2.h"
#include <gtest/gtest.h>
#include <vector>

namespace ww4_core {

TEST(ST2Test, BasicPropertiesAndCalculate) {
  SourceTermST2 source;
  EXPECT_EQ(source.getName(), "ST2");

  std::vector<double> data = {1.0, 2.0};
  source.calculate(data);
  EXPECT_DOUBLE_EQ(data[0], 1.02);
  EXPECT_DOUBLE_EQ(data[1], 2.02);
}

} // namespace ww4_core
