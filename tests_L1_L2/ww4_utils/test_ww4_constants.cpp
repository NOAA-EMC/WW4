/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file test_ww4_constants.cpp
 * @brief Unit tests for WW4 constants in ww4_constants.hpp.
 * @details Verifies the values of mathematical and physical constants.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * @date 2026-04-17
 */

#include "ww4_utils/ww4_constants.hpp"
#include <gtest/gtest.h>

namespace ww4_constants {
namespace testing {

/**
 * @test VerifyMathematicalConstants
 * @brief Ensures all mathematical constants are correctly defined.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Rahul Mahajan
 */
TEST(WW4ConstantsTest, VerifyMathematicalConstants) {
  static_assert(PI == 3.14159265358979323846);
  static_assert(TPI == 2.0 * PI);
  static_assert(HPI == 0.5 * PI);
  static_assert(TPIINV == 1.0 / TPI);
  static_assert(HPIINV == 1.0 / HPI);
  static_assert(RADE == 180.0 / PI);
  static_assert(DERA == PI / 180.0);

  EXPECT_DOUBLE_EQ(PI, 3.14159265358979323846);
  EXPECT_DOUBLE_EQ(TPI, 6.28318530717958647692);
  EXPECT_DOUBLE_EQ(HPI, 1.57079632679489661923);
  EXPECT_NEAR(TPIINV, 0.15915494309189533, 1e-15);
  EXPECT_NEAR(HPIINV, 0.6366197723675813, 1e-15);
  EXPECT_NEAR(RADE, 57.29577951308232, 1e-14);
  EXPECT_NEAR(DERA, 0.017453292519943295, 1e-17);
}

/**
 * @test VerifyPhysicalConstants
 * @brief Ensures all physical constants match WW3 values.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Rahul Mahajan
 */
TEST(WW4ConstantsTest, VerifyPhysicalConstants) {
  static_assert(GRAV == 9.806);
  static_assert(DWAT == 1000.0);
  static_assert(DAIR == 1.225);
  static_assert(NU_AIR == 1.4e-5);
  static_assert(NU_WATER == 1.31e-6);
  static_assert(SED_SG == 2.65);
  static_assert(KAPPA == 0.40);
  static_assert(RADIUS == 4.0e7 / TPI);

  EXPECT_DOUBLE_EQ(GRAV, 9.806);
  EXPECT_DOUBLE_EQ(DWAT, 1000.0);
  EXPECT_DOUBLE_EQ(DAIR, 1.225);
  EXPECT_DOUBLE_EQ(NU_AIR, 1.4e-5);
  EXPECT_DOUBLE_EQ(NU_WATER, 1.31e-6);
  EXPECT_DOUBLE_EQ(SED_SG, 2.65);
  EXPECT_DOUBLE_EQ(KAPPA, 0.40);
  EXPECT_NEAR(RADIUS, 6366197.723675813, 1e-8);
}

/**
 * @test VerifyDerivedConstants
 * @brief Ensures derived constants are correctly calculated.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Rahul Mahajan
 */
TEST(WW4ConstantsTest, VerifyDerivedConstants) {
  EXPECT_DOUBLE_EQ(G2PI3I, 1.0 / (GRAV * GRAV * TPI * TPI * TPI));
  EXPECT_DOUBLE_EQ(G1PI1I, 1.0 / (GRAV * TPI));
}

/**
 * @test VerifyModelConstants
 * @brief Ensures model-specific constants are correctly defined.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Rahul Mahajan
 */
TEST(WW4ConstantsTest, VerifyModelConstants) {
  static_assert(UNDEF == -999.9);
  static_assert(ABMIN == -1.0);
  static_assert(ABMAX == 8.0);
  static_assert(KDMAX == 20.0);
  static_assert(JONSWAP_FACTOR == 0.06175);

  EXPECT_DOUBLE_EQ(UNDEF, -999.9);
  EXPECT_DOUBLE_EQ(ABMIN, -1.0);
  EXPECT_DOUBLE_EQ(ABMAX, 8.0);
  EXPECT_DOUBLE_EQ(KDMAX, 20.0);
  EXPECT_DOUBLE_EQ(JONSWAP_FACTOR, 0.06175);
}

/**
 * @test VerifyConstantsConsistency
 * @brief Ensures mathematical relations between constants are maintained.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Rahul Mahajan
 */
TEST(WW4ConstantsTest, VerifyConstantsConsistency) {
  EXPECT_NEAR(DERA * RADE, 1.0, 1e-15);
  EXPECT_NEAR(TPI * TPIINV, 1.0, 1e-15);
  EXPECT_NEAR(HPI * HPIINV, 1.0, 1e-15);
  EXPECT_NEAR(TPI, 4.0 * HPI, 1e-15);
}

} // namespace testing
} // namespace ww4_constants
