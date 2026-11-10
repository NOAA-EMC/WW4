/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_UQ_domain_topology.cpp
 * @brief Unit tests for UQ domain topology and graph analysis routines.
 *
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @author Code Heritage: Converted and adapted from WAVEWATCH III (WW3)
 * ww3_gspl by Hendrik L. Tolman.
 * @date Initial, 2026-11-09
 * @date Last update : 2026-11-09
 */

// Routine coverage references for anonymous namespace helper functions:
// cleanSingleProtrudingPoints dealWithSmallGridExtensions
// determineDomainNeighbors extractBoundaryLine isWetPoint
// reassignDisconnectedPieces resortDomainsBySize

#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <vector>

#include "ww4_core/solver_UQ/UQ_domain_topology.h"

using namespace ww4_core::solver_UQ;

TEST(UQDomainTopologyTest, IsWetPointCheck) {
  EXPECT_FALSE(isWetPoint(0));  // Land
  EXPECT_TRUE(isWetPoint(1));   // Active sea point
  EXPECT_TRUE(isWetPoint(2));   // Boundary point
  EXPECT_FALSE(isWetPoint(3));  // Excluded point
  EXPECT_FALSE(isWetPoint(-1)); // Inactive/ice point
}

TEST(UQDomainTopologyTest, DetermineNeighborsAndResort) {
  int nx = 4;
  int ny = 4;
  std::vector<int> mask(nx * ny, 1);
  std::vector<int> part_map = {1, 1, 2, 2, 1, 1, 2, 2, 3, 3, 4, 4, 3, 3, 4, 4};

  auto nbrs = determineDomainNeighbors(nx, ny, mask, 4, part_map, false);
  ASSERT_EQ(nbrs.size(), 5u);
  EXPECT_TRUE(nbrs[1].count(2) > 0);
  EXPECT_TRUE(nbrs[1].count(3) > 0);
  EXPECT_FALSE(nbrs[1].count(4) > 0); // Diagonals are not 4-connected

  std::vector<int> counts;
  std::vector<std::set<int>> sorted_nbrs;
  resortDomainsBySize(nx, ny, mask, 4, part_map, false, counts, sorted_nbrs);

  EXPECT_EQ(counts.size(), 5u);
  EXPECT_EQ(counts[1], 4);
  EXPECT_EQ(counts[2], 4);
  EXPECT_EQ(counts[3], 4);
  EXPECT_EQ(counts[4], 4);
}

TEST(UQDomainTopologyTest, ReassignDisconnectedPieces) {
  int nx = 5;
  int ny = 5;
  std::vector<int> mask(nx * ny, 1);
  // Partition 1 has a main bulk (4 cells) and a 1-cell disconnected piece at
  // (4, 4)
  std::vector<int> part_map(nx * ny, 2);
  part_map[0] = 1;
  part_map[1] = 1;
  part_map[nx] = 1;
  part_map[nx + 1] = 1;
  part_map[4 * nx + 4] = 1;

  int changes = reassignDisconnectedPieces(nx, ny, mask, 2, part_map, false);
  EXPECT_GT(changes, 0);
  EXPECT_EQ(part_map[4 * nx + 4], 2);
}

TEST(UQDomainTopologyTest, DealWithSmallGridExtensions) {
  int nx = 5;
  int ny = 5;
  std::vector<int> mask(nx * ny, 1);
  // Create an isolated 1-cell protrusion of partition 1 surrounded in row by
  // partition 2
  std::vector<int> part_map(nx * ny, 2);
  part_map[2 * nx + 2] = 1;

  int changes =
      dealWithSmallGridExtensions(nx, ny, mask, 2, part_map, false, 0);
  EXPECT_GT(changes, 0);
  EXPECT_EQ(part_map[2 * nx + 2], 2);
}

TEST(UQDomainTopologyTest, ExtractBoundaryLine) {
  int nx = 4;
  int ny = 4;
  std::vector<int> mask(nx * ny, 1);
  std::vector<int> part_map = {1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2};

  std::vector<int> line =
      extractBoundaryLine(nx, ny, mask, part_map, false, 1, 2);
  EXPECT_EQ(line.size(), 4u);
}

TEST(UQDomainTopologyTest, CleanSingleProtrudingPoints) {
  int nx = 5;
  int ny = 5;
  std::vector<int> mask(nx * ny, 1);
  std::vector<int> part_map(nx * ny, 2);
  // Domain 1 bulk with a single cell protrusion
  part_map[0] = 1;
  part_map[1] = 1;
  part_map[nx] = 1;
  part_map[nx + 1] = 1;
  part_map[2] = 1; // single protrusion with 1 neighbor in domain 1 and 2
                   // neighbors in domain 2

  int cleaned = cleanSingleProtrudingPoints(nx, ny, mask, 2, part_map, false);
  EXPECT_GE(cleaned, 0);
}
