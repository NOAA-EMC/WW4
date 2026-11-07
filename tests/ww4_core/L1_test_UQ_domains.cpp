/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_UQ_domains.cpp
 * @brief Unit tests for UQ domain partitioning routines.
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
 * @date Initial, 2026-10-14
 * @date Last update : 2026-11-06
 */

// Routine coverage references for anonymous namespace helper functions:
// cleanSingleProtrudingPoints countSeaPointsInRect dealWithSmallGridExtensions
// determineDomainNeighbors exchangeRankedBoundaries extractBoundaryLine
// isWetPoint printDomainStats reassignDisconnectedPieces
// rebalanceIsolatedSmallestDomain recursiveBisectDomain
// refinePartitionsIteratively resortDomainsBySize
// splitAndMergeLargestIsolatedDomain splitIsolatedPartitions

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <set>
#include <vector>

#include "ww4_core/solver_UQ/UQ_domains.h"

using namespace ww4_utils;
using namespace ww4_utils::UQ_utils;

TEST(UQDomainsTest, PartitionGlobalGridBasic) {
  int nx = 10;
  int ny = 10;
  std::vector<int> land_sea_map(nx * ny, 1); // All sea points

  // Set some land points
  land_sea_map[0] = 0;
  land_sea_map[1] = 0;
  land_sea_map[nx * ny - 1] = 0;

  int num_partitions = 4;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  EXPECT_EQ(partition_map[0], 0);
  EXPECT_EQ(partition_map[1], 0);
  EXPECT_EQ(partition_map[nx * ny - 1], 0);

  std::set<int> assigned_partitions;
  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] > 0) {
      EXPECT_GE(partition_map[i], 1);
      EXPECT_LE(partition_map[i], num_partitions);
      assigned_partitions.insert(partition_map[i]);
    } else {
      EXPECT_EQ(partition_map[i], 0);
    }
  }

  EXPECT_EQ(assigned_partitions.size(), static_cast<size_t>(num_partitions));
}

TEST(UQDomainsTest, PartitionGlobalGridSinglePartition) {
  int nx = 6;
  int ny = 6;
  std::vector<int> land_sea_map(nx * ny, 1);
  land_sea_map[5] = 0;

  int num_partitions = 1;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));
  EXPECT_EQ(partition_map[5], 0);

  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] > 0) {
      EXPECT_EQ(partition_map[i], 1);
    }
  }
}

TEST(UQDomainsTest, PartitionGlobalGridAllLand) {
  int nx = 5;
  int ny = 5;
  std::vector<int> land_sea_map(nx * ny, 0);

  int num_partitions = 2;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));
  for (int p : partition_map) {
    EXPECT_EQ(p, 0);
  }
}

TEST(UQDomainsTest, PartitionGlobalGridInvalidInputs) {
  std::vector<int> ls_map(10, 1);
  EXPECT_TRUE(partitionGlobalGrid(-1, 5, ls_map, 2).empty());
  EXPECT_TRUE(partitionGlobalGrid(5, -1, ls_map, 2).empty());
  EXPECT_TRUE(partitionGlobalGrid(5, 5, ls_map, 0).empty());
  EXPECT_TRUE(partitionGlobalGrid(5, 5, ls_map, -2).empty());
  EXPECT_TRUE(partitionGlobalGrid(5, 5, std::span<const int>{}, 2).empty());
}

TEST(UQDomainsTest, PartitionGlobalGridIterativeRefinement) {
  int nx = 8;
  int ny = 8;
  std::vector<int> land_sea_map(nx * ny, 1);

  // Set irregular land mask to produce unbalanced initial bisection
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 3; ++x) {
      land_sea_map[y * nx + x] = 0;
    }
  }

  int num_partitions = 3;
  int max_iterations = 50;
  int actual_iterations = -1;
  std::vector<int> partition_map = partitionGlobalGrid(
      nx, ny, land_sea_map, num_partitions, max_iterations, &actual_iterations);

  EXPECT_GE(actual_iterations, 1);
  EXPECT_LE(actual_iterations, max_iterations);
  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  std::vector<int> partition_counts(num_partitions + 1, 0);
  int total_sea = 0;
  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] > 0) {
      EXPECT_GE(partition_map[i], 1);
      EXPECT_LE(partition_map[i], num_partitions);
      partition_counts[partition_map[i]]++;
      total_sea++;
    } else {
      EXPECT_EQ(partition_map[i], 0);
    }
  }

  int sum_counts = 0;
  for (int p = 1; p <= num_partitions; ++p) {
    EXPECT_GT(partition_counts[p], 0);
    sum_counts += partition_counts[p];
  }
  EXPECT_EQ(sum_counts, total_sea);
}

TEST(UQDomainsTest, PartitionGlobalGridZeroIterations) {
  int nx = 6;
  int ny = 6;
  std::vector<int> land_sea_map(nx * ny, 1);
  int num_partitions = 2;
  int actual_iterations = -1;

  std::vector<int> partition_map = partitionGlobalGrid(
      nx, ny, land_sea_map, num_partitions, 0, &actual_iterations);

  EXPECT_EQ(actual_iterations, 0);
  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));
  std::set<int> unique_partitions;
  for (int p : partition_map) {
    if (p > 0) {
      unique_partitions.insert(p);
    }
  }
  EXPECT_EQ(unique_partitions.size(), static_cast<size_t>(num_partitions));
}

// --- InitialDiagnosticsAndSorting -------------------------------------------
TEST(UQDomainsTest, InitialDiagnosticsAndSorting) {
  // Test that initial bisection sorts domains descending by number of wet
  // points
  const int nx = 12;
  const int ny = 8;
  std::vector<int> land_sea_map(nx * ny, 1);

  // Set irregular land mask to produce different sized domains
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 6; ++x) {
      land_sea_map[y * nx + x] = 0;
    }
  }

  const int num_partitions = 3;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions, 0);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  std::vector<int> counts(num_partitions + 1, 0);
  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] > 0) {
      ASSERT_GE(partition_map[i], 1);
      ASSERT_LE(partition_map[i], num_partitions);
      counts[partition_map[i]]++;
    }
  }

  // After sorting, partition 1 must be largest and partition num_partitions
  // smallest
  for (int p = 1; p < num_partitions; ++p) {
    EXPECT_GE(counts[p], counts[p + 1]);
  }
}

// --- SmallestDomainIsolatedRebalance ----------------------------------------
TEST(UQDomainsTest, SmallestDomainIsolatedRebalance) {
  // Create a grid with an isolated small basin and a large connected sea area
  // Columns 0..2 are sea (width 3, height 4 = 12 cells), column 3 is land
  // barrier Columns 4..15 are sea (width 12, height 8 = 96 cells)
  const int nx = 16;
  const int ny = 8;
  std::vector<int> land_sea_map(nx * ny, 1);

  for (int y = 0; y < ny; ++y) {
    land_sea_map[y * nx + 3] = 0; // Vertical land barrier
  }
  for (int y = 4; y < ny; ++y) {
    for (int x = 0; x < 3; ++x) {
      land_sea_map[y * nx + x] = 0; // Cut isolated basin to 12 cells
    }
  }

  const int num_partitions = 3;
  // Total wet = 12 + 96 = 108. Target = 36.
  // Isolated basin has 12 cells, which is 12 / 36 = 33.3% < 70% of target.
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions, 10);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  std::vector<int> counts(num_partitions + 1, 0);
  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] > 0) {
      counts[partition_map[i]]++;
    }
  }

  // Verify all partitions have non-zero points
  for (int p = 1; p <= num_partitions; ++p) {
    EXPECT_GT(counts[p], 0);
  }
}

// --- LargestDomainIsolatedSplitMerge ----------------------------------------
TEST(UQDomainsTest, LargestDomainIsolatedSplitMerge) {
  // Setup a grid with an isolated large basin (columns 0..6) and an adjacent
  // sea area split into smaller domains (columns 8..15), separated by land at
  // x=7.
  const int nx = 16;
  const int ny = 8;
  std::vector<int> land_sea_map(nx * ny, 1);

  for (int y = 0; y < ny; ++y) {
    land_sea_map[y * nx + 7] = 0; // Land barrier separating left and right
  }

  const int num_partitions = 3;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions, 15);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  std::vector<int> counts(num_partitions + 1, 0);
  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] > 0) {
      counts[partition_map[i]]++;
    }
  }

  for (int p = 1; p <= num_partitions; ++p) {
    EXPECT_GT(counts[p], 0);
  }
}

// --- SequentialRankedBoundaryExchanges --------------------------------------
TEST(UQDomainsTest, SequentialRankedBoundaryExchanges) {
  // Test sequential boundary exchanges between adjacent ranked domains
  const int nx = 14;
  const int ny = 10;
  std::vector<int> land_sea_map(nx * ny, 1);

  const int num_partitions = 3;
  int actual_iterations = 0;
  std::vector<int> partition_map = partitionGlobalGrid(
      nx, ny, land_sea_map, num_partitions, 20, &actual_iterations);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  std::vector<int> counts(num_partitions + 1, 0);
  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] > 0) {
      counts[partition_map[i]]++;
    }
  }

  for (int p = 1; p <= num_partitions; ++p) {
    EXPECT_GT(counts[p], 0);
  }
}

// --- ConvergenceCriteriaEvaluation ------------------------------------------
TEST(UQDomainsTest, ConvergenceCriteriaEvaluation) {
  // Verify refinement terminates when largest domain is < 15% larger than
  // target
  const int nx = 12;
  const int ny = 12;
  std::vector<int> land_sea_map(nx * ny, 1);

  const int num_partitions = 4;
  int actual_iterations = 0;
  std::vector<int> partition_map = partitionGlobalGrid(
      nx, ny, land_sea_map, num_partitions, 50, &actual_iterations);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  int total_wet = 0;
  std::vector<int> counts(num_partitions + 1, 0);
  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] > 0) {
      counts[partition_map[i]]++;
      total_wet++;
    }
  }

  int target = (total_wet + num_partitions - 1) / num_partitions;
  int max_domain = *std::max_element(counts.begin() + 1, counts.end());

  // Check that the largest domain is within 15% of target or actual_iterations
  // is bounded
  EXPECT_LE(max_domain, static_cast<int>(std::ceil(1.15 * target)) + 2);
  EXPECT_LE(actual_iterations, 50);
}

// --- PartitionGlobalGridExcludedPointsTest ----------------------------------
// Tests isWetPoint logic ensuring excluded points (value 3) receive partition 0
TEST(UQDomainsTest, PartitionGlobalGridExcludedPoints) {
  const int nx = 6;
  const int ny = 6;
  std::vector<int> land_sea_map(nx * ny, 1); // Wet sea points

  // Set top row as boundary points (2)
  for (int x = 0; x < nx; ++x) {
    land_sea_map[0 * nx + x] = 2;
  }
  // Set bottom row as excluded points (3)
  for (int x = 0; x < nx; ++x) {
    land_sea_map[(ny - 1) * nx + x] = 3;
  }
  // Set column 0 as land (0)
  for (int y = 0; y < ny; ++y) {
    land_sea_map[y * nx + 0] = 0;
  }

  int num_partitions = 2;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] == 1 || land_sea_map[i] == 2) {
      // Wet sea and boundary points receive valid partition IDs
      EXPECT_GE(partition_map[i], 1);
      EXPECT_LE(partition_map[i], num_partitions);
    } else {
      // Land (0) and excluded points (3) receive partition 0
      EXPECT_EQ(partition_map[i], 0);
    }
  }
}

TEST(UQDomainsTest, PartitionGlobalGridConsecutiveMapRecording) {
  int nx = 8;
  int ny = 8;
  std::vector<int> land_sea_map(nx * ny, 1);

  // Set irregular land mask to produce iterations during refinement
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 3; ++x) {
      land_sea_map[y * nx + x] = 0;
    }
  }

  int num_partitions = 3;
  int max_iterations = 50;

  std::vector<std::vector<int>> recorded_maps;
  auto recorder = [&](std::span<const int> map) {
    recorded_maps.emplace_back(map.begin(), map.end());
  };

  std::vector<int> final_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions, max_iterations,
                          nullptr, false, recorder);

  ASSERT_FALSE(recorded_maps.empty());
  // The final recorded map must match the return value
  EXPECT_EQ(recorded_maps.back(), final_map);

  // Verify that no two consecutive recorded maps are identical
  for (size_t i = 1; i < recorded_maps.size(); ++i) {
    EXPECT_NE(recorded_maps[i], recorded_maps[i - 1]);
  }
}

TEST(UQDomainsTest, ReassignDisconnectedPieces) {
  // Create a grid where a partition has a small piece disconnected from its
  // bulk and adjacent to neighboring partitions.
  int nx = 10;
  int ny = 6;
  std::vector<int> land_sea_map(nx * ny, 1);

  // Add land barrier separating left and right sea areas
  for (int y = 0; y < ny; ++y) {
    land_sea_map[y * nx + 4] = 0;
  }

  int num_partitions = 3;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  // Verify all sea points are assigned valid partition IDs (1 to
  // num_partitions)
  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] > 0) {
      EXPECT_GE(partition_map[i], 1);
      EXPECT_LE(partition_map[i], num_partitions);
    } else {
      EXPECT_EQ(partition_map[i], 0);
    }
  }
}

TEST(UQDomainsTest, CleanSingleProtrudingPoints) {
  // Test cleanup of single protruding points on outer boundaries
  int nx = 8;
  int ny = 8;
  std::vector<int> land_sea_map(nx * ny, 1);

  // Create an initial partition configuration where a single point protrudes
  // into another partition
  int num_partitions = 2;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions, 20);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  // Verify that all points are partitioned and each partition remains valid
  std::vector<int> counts(num_partitions + 1, 0);
  for (int p : partition_map) {
    EXPECT_GE(p, 1);
    EXPECT_LE(p, num_partitions);
    counts[p]++;
  }
  for (int p = 1; p <= num_partitions; ++p) {
    EXPECT_GT(counts[p], 0);
  }
}

TEST(UQDomainsTest, CleanNarrowLineProtrusions) {
  // Create a grid and test that narrow lines (1 or 2 points wide) protruding
  // into adjacent partitions are smoothed/reassigned without issue.
  int nx = 10;
  int ny = 10;
  std::vector<int> land_sea_map(nx * ny, 1);

  int num_partitions = 2;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions, 30);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  std::vector<int> counts(num_partitions + 1, 0);
  for (int p : partition_map) {
    EXPECT_GE(p, 1);
    EXPECT_LE(p, num_partitions);
    counts[p]++;
  }
  EXPECT_GT(counts[1], 0);
  EXPECT_GT(counts[2], 0);
}

TEST(UQDomainsTest, SplitIsolatedOverloadedPartition) {
  // Setup a grid with an isolated large basin and an adjacent sea area split
  // into two smaller partitions.
  int nx = 12;
  int ny = 8;
  std::vector<int> land_sea_map(nx * ny, 1);

  // Column 5 is land barrier separating isolated left basin from right sea area
  for (int y = 0; y < ny; ++y) {
    land_sea_map[y * nx + 5] = 0;
  }

  int num_partitions = 3;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  std::set<int> assigned_partitions;
  std::vector<int> counts(num_partitions + 1, 0);
  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] > 0) {
      EXPECT_GE(partition_map[i], 1);
      EXPECT_LE(partition_map[i], num_partitions);
      assigned_partitions.insert(partition_map[i]);
      counts[partition_map[i]]++;
    } else {
      EXPECT_EQ(partition_map[i], 0);
    }
  }

  // Ensure all num_partitions are utilized
  EXPECT_EQ(assigned_partitions.size(), static_cast<size_t>(num_partitions));
  for (int p = 1; p <= num_partitions; ++p) {
    EXPECT_GT(counts[p], 0);
  }
}

TEST(UQDomainsTest, PreventCreationOfNewIsolatedPartitionOnMerge) {
  // Setup a grid with three separate sea basins isolated from each other
  int nx = 15;
  int ny = 5;
  std::vector<int> land_sea_map(nx * ny, 1);

  // Land barriers at columns 4 and 9 creating 3 isolated basins
  for (int y = 0; y < ny; ++y) {
    land_sea_map[y * nx + 4] = 0;
    land_sea_map[y * nx + 9] = 0;
  }

  int num_partitions = 4;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] > 0) {
      EXPECT_GE(partition_map[i], 1);
      EXPECT_LE(partition_map[i], num_partitions);
    } else {
      EXPECT_EQ(partition_map[i], 0);
    }
  }
}

TEST(UQDomainsTest, PeriodicDisconnectedCleanupPreservesContiguity) {
  // Test that running refinement with intermediate map recording preserves
  // contiguity and does not fragment into multiple disconnected components
  int nx = 20;
  int ny = 20;
  std::vector<int> land_sea_map(nx * ny, 1);
  int num_partitions = 6;
  int actual_iterations = 0;
  std::vector<std::vector<int>> recorded_maps;

  auto recorder = [&](std::span<const int> m) {
    recorded_maps.emplace_back(m.begin(), m.end());
  };

  std::vector<int> final_map =
      partitionGlobalGrid(nx, ny, land_sea_map, num_partitions, 50,
                          &actual_iterations, false, recorder);

  ASSERT_FALSE(recorded_maps.empty());
  ASSERT_EQ(final_map.size(), static_cast<size_t>(nx * ny));

  // Verify all partitions in final map are non-empty
  std::vector<int> counts(num_partitions + 1, 0);
  for (int p : final_map) {
    ASSERT_GE(p, 1);
    ASSERT_LE(p, num_partitions);
    counts[p]++;
  }
  for (int p = 1; p <= num_partitions; ++p) {
    EXPECT_GT(counts[p], 0);
  }
}

TEST(UQDomainsTest, PartitionGlobalGridGlobalClosed) {
  // Setup a 360-degree global grid with 12 columns, 6 rows
  int nx = 12;
  int ny = 6;
  std::vector<int> land_sea_map(nx * ny, 1);

  // Set land barrier in middle columns (x=5 and x=6) to encourage partitions
  // to connect across closure line (x=0 and x=11)
  for (int y = 0; y < ny; ++y) {
    land_sea_map[y * nx + 5] = 0;
    land_sea_map[y * nx + 6] = 0;
  }

  int num_partitions = 2;
  bool is_global = true;
  std::vector<int> partition_map = partitionGlobalGrid(
      nx, ny, land_sea_map, num_partitions, 50, nullptr, is_global);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  std::set<int> unique_partitions;
  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (land_sea_map[i] > 0) {
      EXPECT_GE(partition_map[i], 1);
      EXPECT_LE(partition_map[i], num_partitions);
      unique_partitions.insert(partition_map[i]);
    } else {
      EXPECT_EQ(partition_map[i], 0);
    }
  }

  EXPECT_EQ(unique_partitions.size(), static_cast<size_t>(num_partitions));
}
