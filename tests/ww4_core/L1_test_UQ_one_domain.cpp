/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_UQ_one_domain.cpp
 * @brief Unit tests for UQ single subdomain extraction routine
 * (extractOneDomain).
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
 * @date Initial, 2026-10-29
 * @date Last update : 2026-11-09
 */

// Routine coverage references for anonymous namespace helper functions:
// extractOneDomain

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "ww4_core/solver_UQ/UQ_domains.h"
#include "ww4_core/solver_UQ/UQ_one_domain.h"
#include "ww4_utils/spatial_grid.h"

using namespace ww4_utils;
using namespace ww4_core::solver_UQ;

TEST(UQOneDomainTest, ExtractOneDomainBasic) {
  int nx = 10;
  int ny = 10;
  FullGrid full;
  full.nx = nx;
  full.ny = ny;
  full.landMask.assign(nx * ny, 1);
  full.landMask[0] = 0; // land
  full.shared.landMask = full.landMask;

  // 1D Coordinates
  full.xCoordinates.resize(nx);
  for (int x = 0; x < nx; ++x) {
    full.xCoordinates[x] = -100.0 + x * 0.5;
  }
  full.shared.xCoordinates = full.xCoordinates;
  full.yCoordinates.resize(ny);
  for (int y = 0; y < ny; ++y) {
    full.yCoordinates[y] = 20.0 + y * 0.5;
  }
  full.shared.yCoordinates = full.yCoordinates;

  // Topobathy and subgrid obstruction
  full.topobathy.resize(nx * ny);
  full.shared.obstructionX.resize(nx * ny, 1.0);
  full.shared.obstructionY.resize(nx * ny, 1.0);
  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      full.topobathy[y * nx + x] = -10.0 - (y * nx + x);
    }
  }
  full.shared.topobathy = full.topobathy;

  int num_partitions = 2;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, full.landMask, num_partitions);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  // Extract Partition 1 with halo = 1
  std::vector<int> sliced_map;
  LocalGridPartition red1 =
      extractOneDomain(full, partition_map, 1, 1, false, &sliced_map);

  EXPECT_EQ(red1.rankId, 0);
  EXPECT_EQ(red1.haloSizeX, 1u);
  EXPECT_EQ(red1.haloSizeY, 1u);
  EXPECT_GT(red1.localNx, 0u);
  EXPECT_GT(red1.localNy, 0u);
  EXPECT_LE(red1.localNx, static_cast<std::size_t>(nx));
  EXPECT_LE(red1.localNy, static_cast<std::size_t>(ny));

  // Check 1D coordinates size in reduced grid
  EXPECT_EQ(red1.xCoordinates.size(), red1.localNx);
  EXPECT_EQ(red1.yCoordinates.size(), red1.localNy);

  // Verify coordinate values match global slice
  // Find global bounds from localToGlobalMap
  ASSERT_FALSE(red1.localToGlobalMap.empty());
  std::size_t g_origin = red1.localToGlobalMap[0];
  int gx_min = static_cast<int>(g_origin % nx);
  int gy_min = static_cast<int>(g_origin / nx);

  for (std::size_t x_loc = 0; x_loc < red1.localNx; ++x_loc) {
    int gx = gx_min + static_cast<int>(x_loc);
    EXPECT_DOUBLE_EQ(red1.xCoordinates[x_loc], full.xCoordinates[gx]);
  }
  for (std::size_t y_loc = 0; y_loc < red1.localNy; ++y_loc) {
    int gy = gy_min + static_cast<int>(y_loc);
    EXPECT_DOUBLE_EQ(red1.yCoordinates[y_loc], full.yCoordinates[gy]);
  }

  // Verify field slicing values (topobathy, landMask, sliced partition map)
  size_t loc_size = red1.localNx * red1.localNy;
  ASSERT_EQ(red1.landMask.size(), loc_size);
  ASSERT_EQ(sliced_map.size(), loc_size);
  ASSERT_EQ(red1.topobathy.size(), loc_size);
  ASSERT_EQ(red1.shared.obstructionX.size(), loc_size);
  ASSERT_EQ(red1.shared.obstructionY.size(), loc_size);

  for (std::size_t y_loc = 0; y_loc < red1.localNy; ++y_loc) {
    int gy = gy_min + static_cast<int>(y_loc);
    for (std::size_t x_loc = 0; x_loc < red1.localNx; ++x_loc) {
      int gx = gx_min + static_cast<int>(x_loc);
      int g_idx = gy * nx + gx;
      std::size_t l_idx = y_loc * red1.localNx + x_loc;

      EXPECT_EQ(sliced_map[l_idx], partition_map[g_idx]);
      EXPECT_EQ(red1.landMask[l_idx], full.landMask[g_idx]);
      EXPECT_DOUBLE_EQ(red1.topobathy[l_idx], full.topobathy[g_idx]);
      EXPECT_DOUBLE_EQ(red1.shared.obstructionX[l_idx],
                       full.shared.obstructionX[g_idx]);
      EXPECT_DOUBLE_EQ(red1.shared.obstructionY[l_idx],
                       full.shared.obstructionY[g_idx]);
    }
  }
}

TEST(UQOneDomainTest, ExtractOneDomain2DCoordinates) {
  int nx = 6;
  int ny = 6;
  FullGrid full;
  full.nx = nx;
  full.ny = ny;
  full.landMask.assign(nx * ny, 1);
  full.shared.landMask = full.landMask;

  // 2D curvilinear coordinates
  full.xCoordinates.resize(nx * ny);
  full.yCoordinates.resize(nx * ny);
  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      int idx = y * nx + x;
      full.xCoordinates[idx] = 10.0 + x * 0.1 + y * 0.01;
      full.yCoordinates[idx] = 40.0 + y * 0.1 + x * 0.01;
    }
  }
  full.shared.xCoordinates = full.xCoordinates;
  full.shared.yCoordinates = full.yCoordinates;

  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, full.landMask, 2);

  LocalGridPartition red =
      extractOneDomain(full, partition_map, 1, 0); // zero halo

  size_t loc_size = red.localNx * red.localNy;
  ASSERT_EQ(red.xCoordinates.size(), loc_size);
  ASSERT_EQ(red.yCoordinates.size(), loc_size);

  std::size_t g_origin = red.localToGlobalMap[0];
  int gx_min = static_cast<int>(g_origin % nx);
  int gy_min = static_cast<int>(g_origin / nx);

  for (std::size_t y_loc = 0; y_loc < red.localNy; ++y_loc) {
    int gy = gy_min + static_cast<int>(y_loc);
    for (std::size_t x_loc = 0; x_loc < red.localNx; ++x_loc) {
      int gx = gx_min + static_cast<int>(x_loc);
      int g_idx = gy * nx + gx;
      std::size_t l_idx = y_loc * red.localNx + x_loc;

      EXPECT_DOUBLE_EQ(red.xCoordinates[l_idx], full.xCoordinates[g_idx]);
      EXPECT_DOUBLE_EQ(red.yCoordinates[l_idx], full.yCoordinates[g_idx]);
    }
  }
}

TEST(UQOneDomainTest, ExtractOneDomainHaloClamping) {
  int nx = 8;
  int ny = 8;
  FullGrid full;
  full.nx = nx;
  full.ny = ny;
  full.landMask.assign(nx * ny, 1);
  full.shared.landMask = full.landMask;

  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, full.landMask, 2);

  // Large halo width (e.g. 20) should be clamped to grid boundaries
  LocalGridPartition red = extractOneDomain(full, partition_map, 1, 20);

  EXPECT_EQ(red.localNx, static_cast<std::size_t>(nx));
  EXPECT_EQ(red.localNy, static_cast<std::size_t>(ny));
  EXPECT_EQ(red.localToGlobalMap.front(), 0u);
  EXPECT_EQ(red.localToGlobalMap.back(), static_cast<std::size_t>(nx * ny - 1));
}

TEST(UQOneDomainTest, ExtractOneDomainGlobalClosedWrapping) {
  int nx = 10;
  int ny = 4;
  FullGrid full;
  full.nx = nx;
  full.ny = ny;
  RectangularGridData rect;
  rect.isGlobal = true;
  full.specific = rect;
  full.landMask.assign(nx * ny, 1);
  full.shared.landMask = full.landMask;

  // Define a partition map where Partition 1 occupies x = 0, 1 and x = 8, 9
  std::vector<int> partition_map(nx * ny, 2);
  for (int y = 0; y < ny; ++y) {
    partition_map[y * nx + 0] = 1;
    partition_map[y * nx + 1] = 1;
    partition_map[y * nx + 8] = 1;
    partition_map[y * nx + 9] = 1;
  }

  // 1D longitude array
  full.xCoordinates.resize(nx);
  for (int x = 0; x < nx; ++x) {
    full.xCoordinates[x] = x * 36.0;
  }
  full.shared.xCoordinates = full.xCoordinates;

  // Extract Partition 1 with halo = 1 across closure line
  std::vector<int> sliced_map;
  LocalGridPartition red =
      extractOneDomain(full, partition_map, 1, 1, true, &sliced_map);

  // Occupied span is {8, 9, 0, 1} (4 cols). Halo = 1 expands to {7, 8, 9, 0, 1,
  // 2} (6 cols)
  EXPECT_EQ(red.localNx, 6u);
  EXPECT_EQ(red.localNy, static_cast<std::size_t>(ny));

  // Check 1D longitudes for wrapped local grid
  ASSERT_EQ(red.xCoordinates.size(), 6u);
  EXPECT_DOUBLE_EQ(red.xCoordinates[0], full.xCoordinates[7]);
  EXPECT_DOUBLE_EQ(red.xCoordinates[1], full.xCoordinates[8]);
  EXPECT_DOUBLE_EQ(red.xCoordinates[2], full.xCoordinates[9]);
  EXPECT_DOUBLE_EQ(red.xCoordinates[3], full.xCoordinates[0]);
  EXPECT_DOUBLE_EQ(red.xCoordinates[4], full.xCoordinates[1]);
  EXPECT_DOUBLE_EQ(red.xCoordinates[5], full.xCoordinates[2]);

  // Check partition_map slicing for wrapped local grid
  for (std::size_t y_loc = 0; y_loc < red.localNy; ++y_loc) {
    for (std::size_t x_loc = 0; x_loc < red.localNx; ++x_loc) {
      int gx = static_cast<int>((7 + x_loc) % nx);
      int g_idx = static_cast<int>(y_loc) * nx + gx;
      std::size_t l_idx = y_loc * red.localNx + x_loc;
      EXPECT_EQ(sliced_map[l_idx], partition_map[g_idx]);
    }
  }
}

TEST(UQOneDomainTest, ExtractOneDomainInvalidPartitionOrGrid) {
  FullGrid full;
  full.nx = 5;
  full.ny = 5;
  full.landMask.assign(25, 1);
  full.shared.landMask = full.landMask;

  std::vector<int> partition_map(25, 1);

  // Non-existent partition ID
  LocalGridPartition red_invalid = extractOneDomain(full, partition_map, 99, 1);
  EXPECT_EQ(red_invalid.localNx, 0u);
  EXPECT_EQ(red_invalid.localNy, 0u);
  EXPECT_TRUE(red_invalid.landMask.empty());

  // Mismatched partition_map size
  std::vector<int> short_map(10, 1);
  LocalGridPartition red_short = extractOneDomain(full, short_map, 1, 1);
  EXPECT_EQ(red_short.localNx, 0u);
  EXPECT_EQ(red_short.localNy, 0u);

  // Invalid nx or ny
  FullGrid bad_full = full;
  bad_full.nx = 0;
  LocalGridPartition red_bad_dim =
      extractOneDomain(bad_full, partition_map, 1, 1);
  EXPECT_EQ(red_bad_dim.localNx, 0u);
}
