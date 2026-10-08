/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_scheme_factory.cpp
 * @brief Unit tests for the SchemeFactory class.
 * @details Verifies the instantiation of numerical solvers and source term
 *          schemes using SchemeFactory.
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
 * @date Initial, 2026-06-24
 * @date Last update : 2026-09-29
 */

#include "ww4_core/scheme_factory.h"
#include <gtest/gtest.h>

namespace ww4_core {

TEST(SchemeFactoryTest, CreateSolverUQ) {
  auto solver = SchemeFactory::createSolver("SolverUQ");
  ASSERT_NE(solver, nullptr);
  EXPECT_EQ(solver->getName(), "UQ");
}

TEST(SchemeFactoryTest, CreateSolverTriangular) {
  auto solver = SchemeFactory::createSolver("SolverTriangular");
  ASSERT_NE(solver, nullptr);
  EXPECT_EQ(solver->getName(), "Triangular");
}

TEST(SchemeFactoryTest, CreateSolverSMC) {
  auto solver = SchemeFactory::createSolver("SolverSMC");
  ASSERT_NE(solver, nullptr);
  EXPECT_EQ(solver->getName(), "SMC");
}

TEST(SchemeFactoryTest, CreateSourceComputeAllSources) {
  auto scheme = SchemeFactory::createSourceTerm("compute_all_sources");
  ASSERT_NE(scheme, nullptr);
  EXPECT_EQ(scheme->getName(), "ComputeAllSources");
}

TEST(SchemeFactoryTest, CreateSubSourceTerms) {
  auto st1 = SchemeFactory::createSourceTerm("st1");
  ASSERT_NE(st1, nullptr);
  EXPECT_EQ(st1->getName(), "ST1");

  auto st2 = SchemeFactory::createSourceTerm("st2");
  ASSERT_NE(st2, nullptr);
  EXPECT_EQ(st2->getName(), "ST2");

  auto st4 = SchemeFactory::createSourceTerm("ST4");
  ASSERT_NE(st4, nullptr);
  EXPECT_EQ(st4->getName(), "ST4");

  auto st6 = SchemeFactory::createSourceTerm("ST6");
  ASSERT_NE(st6, nullptr);
  EXPECT_EQ(st6->getName(), "ST6");

  auto nl1 = SchemeFactory::createSourceTerm("nl1");
  ASSERT_NE(nl1, nullptr);
  EXPECT_EQ(nl1->getName(), "NL1");

  auto nl2 = SchemeFactory::createSourceTerm("nl2");
  ASSERT_NE(nl2, nullptr);
  EXPECT_EQ(nl2->getName(), "NL2");

  auto nl3 = SchemeFactory::createSourceTerm("NL3");
  ASSERT_NE(nl3, nullptr);
  EXPECT_EQ(nl3->getName(), "NL3");

  auto ln1 = SchemeFactory::createSourceTerm("LN1");
  ASSERT_NE(ln1, nullptr);
  EXPECT_EQ(ln1->getName(), "LN1");

  auto bt1 = SchemeFactory::createSourceTerm("BT1");
  ASSERT_NE(bt1, nullptr);
  EXPECT_EQ(bt1->getName(), "BT1");

  auto bt4 = SchemeFactory::createSourceTerm("BT4");
  ASSERT_NE(bt4, nullptr);
  EXPECT_EQ(bt4->getName(), "BT4");
}

TEST(SchemeFactoryTest, CreateUnknownSolver) {
  EXPECT_THROW(SchemeFactory::createSolver("UNKNOWN"), std::invalid_argument);
}

TEST(SchemeFactoryTest, CreateUnknownSource) {
  EXPECT_THROW(SchemeFactory::createSourceTerm("UNKNOWN"),
               std::invalid_argument);
}

} // namespace ww4_core
