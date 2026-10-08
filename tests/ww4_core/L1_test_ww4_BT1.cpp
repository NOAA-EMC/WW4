/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_ww4_BT1.cpp
 * @brief Unit tests for BT1 bottom friction source term.
 * @details Verifies the behavior and interface compliance of SourceTermBT1.
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

#include "ww4_core/source_terms/ww4_BT1/ww4_BT1.h"
#include <gtest/gtest.h>
#include <vector>

namespace ww4_core {

TEST(BT1Test, BasicPropertiesAndCalculate) {
  SourceTermBT1 source;
  EXPECT_EQ(source.getName(), "BT1");

  std::vector<double> data = {1.0, 2.0};
  source.calculate(data);
  EXPECT_DOUBLE_EQ(data[0], 0.999);
  EXPECT_DOUBLE_EQ(data[1], 1.999);
}

} // namespace ww4_core
