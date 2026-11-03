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
 * @date Last update : 2026-11-03
 */

// Routine coverage references for anonymous namespace helper functions:
// extractOneDomain

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "ww4_core/solver_UQ/UQ_domains.h"
#include "ww4_core/solver_UQ/UQ_one_domain.h"

using namespace ww4_utils;
using namespace ww4_utils::UQ_utils;

TEST(UQOneDomainTest, ExtractOneDomainBasic) {
  int nx = 10;
  int ny = 10;
  FullGrid full;
  full.nx = nx;
  full.ny = ny;
  full.land_sea_map.assign(nx * ny, 1);
  full.land_sea_map[0] = 0; // land

  // 1D Coordinates
  full.longitude.resize(nx);
  for (int x = 0; x < nx; ++x) {
    full.longitude[x] = -100.0 + x * 0.5;
  }
  full.latitude.resize(ny);
  for (int y = 0; y < ny; ++y) {
    full.latitude[y] = 20.0 + y * 0.5;
  }

  // Topobathy and subgrid obstruction
  full.topobathy.resize(nx * ny);
  full.subgrid_obs_x.resize(nx * ny, 1.0);
  full.subgrid_obs_y.resize(nx * ny, 1.0);
  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      full.topobathy[y * nx + x] = -10.0 - (y * nx + x);
    }
  }

  int num_partitions = 2;
  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, full.land_sea_map, num_partitions);

  ASSERT_EQ(partition_map.size(), static_cast<size_t>(nx * ny));

  // Extract Partition 1 with halo = 1
  ReducedGrid red1 = extractOneDomain(full, partition_map, 1, 1);

  EXPECT_EQ(red1.target_partition, 1);
  EXPECT_EQ(red1.halo_width, 1);
  EXPECT_GT(red1.nx_local, 0);
  EXPECT_GT(red1.ny_local, 0);
  EXPECT_LE(red1.nx_local, nx);
  EXPECT_LE(red1.ny_local, ny);

  // Check 1D coordinates size in reduced grid
  EXPECT_EQ(red1.longitude.size(), static_cast<size_t>(red1.nx_local));
  EXPECT_EQ(red1.latitude.size(), static_cast<size_t>(red1.ny_local));

  // Verify coordinate values match global slice
  for (int x_loc = 0; x_loc < red1.nx_local; ++x_loc) {
    int gx = red1.global_x_min + x_loc;
    EXPECT_DOUBLE_EQ(red1.longitude[x_loc], full.longitude[gx]);
  }
  for (int y_loc = 0; y_loc < red1.ny_local; ++y_loc) {
    int gy = red1.global_y_min + y_loc;
    EXPECT_DOUBLE_EQ(red1.latitude[y_loc], full.latitude[gy]);
  }

  // Verify field slicing values (topobathy, land_sea_map, partition_map)
  size_t loc_size = static_cast<size_t>(red1.nx_local * red1.ny_local);
  ASSERT_EQ(red1.land_sea_map.size(), loc_size);
  ASSERT_EQ(red1.partition_map.size(), loc_size);
  ASSERT_EQ(red1.topobathy.size(), loc_size);
  ASSERT_EQ(red1.subgrid_obs_x.size(), loc_size);
  ASSERT_EQ(red1.subgrid_obs_y.size(), loc_size);

  for (int y_loc = 0; y_loc < red1.ny_local; ++y_loc) {
    int gy = red1.global_y_min + y_loc;
    for (int x_loc = 0; x_loc < red1.nx_local; ++x_loc) {
      int gx = red1.global_x_min + x_loc;
      int g_idx = gy * nx + gx;
      int l_idx = y_loc * red1.nx_local + x_loc;

      EXPECT_EQ(red1.partition_map[l_idx], partition_map[g_idx]);
      EXPECT_EQ(red1.land_sea_map[l_idx], full.land_sea_map[g_idx]);
      EXPECT_DOUBLE_EQ(red1.topobathy[l_idx], full.topobathy[g_idx]);
      EXPECT_DOUBLE_EQ(red1.subgrid_obs_x[l_idx], full.subgrid_obs_x[g_idx]);
      EXPECT_DOUBLE_EQ(red1.subgrid_obs_y[l_idx], full.subgrid_obs_y[g_idx]);
    }
  }
}

TEST(UQOneDomainTest, ExtractOneDomain2DCoordinates) {
  int nx = 6;
  int ny = 6;
  FullGrid full;
  full.nx = nx;
  full.ny = ny;
  full.land_sea_map.assign(nx * ny, 1);

  // 2D curvilinear coordinates
  full.longitude.resize(nx * ny);
  full.latitude.resize(nx * ny);
  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      int idx = y * nx + x;
      full.longitude[idx] = 10.0 + x * 0.1 + y * 0.01;
      full.latitude[idx] = 40.0 + y * 0.1 + x * 0.01;
    }
  }

  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, full.land_sea_map, 2);

  ReducedGrid red = extractOneDomain(full, partition_map, 1, 0); // zero halo

  size_t loc_size = static_cast<size_t>(red.nx_local * red.ny_local);
  ASSERT_EQ(red.longitude.size(), loc_size);
  ASSERT_EQ(red.latitude.size(), loc_size);

  for (int y_loc = 0; y_loc < red.ny_local; ++y_loc) {
    int gy = red.global_y_min + y_loc;
    for (int x_loc = 0; x_loc < red.nx_local; ++x_loc) {
      int gx = red.global_x_min + x_loc;
      int g_idx = gy * nx + gx;
      int l_idx = y_loc * red.nx_local + x_loc;

      EXPECT_DOUBLE_EQ(red.longitude[l_idx], full.longitude[g_idx]);
      EXPECT_DOUBLE_EQ(red.latitude[l_idx], full.latitude[g_idx]);
    }
  }
}

TEST(UQOneDomainTest, ExtractOneDomainHaloClamping) {
  int nx = 8;
  int ny = 8;
  FullGrid full;
  full.nx = nx;
  full.ny = ny;
  full.land_sea_map.assign(nx * ny, 1);

  std::vector<int> partition_map =
      partitionGlobalGrid(nx, ny, full.land_sea_map, 2);

  // Large halo width (e.g. 20) should be clamped to grid boundaries
  ReducedGrid red = extractOneDomain(full, partition_map, 1, 20);

  EXPECT_EQ(red.global_x_min, 0);
  EXPECT_EQ(red.global_y_min, 0);
  EXPECT_EQ(red.global_x_max, nx - 1);
  EXPECT_EQ(red.global_y_max, ny - 1);
  EXPECT_EQ(red.nx_local, nx);
  EXPECT_EQ(red.ny_local, ny);
}

TEST(UQOneDomainTest, ExtractOneDomainGlobalClosedWrapping) {
  int nx = 10;
  int ny = 4;
  FullGrid full;
  full.nx = nx;
  full.ny = ny;
  full.is_global = true;
  full.land_sea_map.assign(nx * ny, 1);

  // Define a partition map where Partition 1 occupies x = 0, 1 and x = 8, 9
  std::vector<int> partition_map(nx * ny, 2);
  for (int y = 0; y < ny; ++y) {
    partition_map[y * nx + 0] = 1;
    partition_map[y * nx + 1] = 1;
    partition_map[y * nx + 8] = 1;
    partition_map[y * nx + 9] = 1;
  }

  // 1D longitude array
  full.longitude.resize(nx);
  for (int x = 0; x < nx; ++x) {
    full.longitude[x] = x * 36.0;
  }

  // Extract Partition 1 with halo = 1 across closure line
  ReducedGrid red = extractOneDomain(full, partition_map, 1, 1, true);

  // Occupied span is {8, 9, 0, 1} (4 cols). Halo = 1 expands to {7, 8, 9, 0, 1,
  // 2} (6 cols)
  EXPECT_EQ(red.nx_local, 6);
  EXPECT_EQ(red.ny_local, ny);
  EXPECT_EQ(red.global_x_min, 7);
  EXPECT_EQ(red.global_x_max, 2);

  // Check 1D longitudes for wrapped local grid
  ASSERT_EQ(red.longitude.size(), 6u);
  EXPECT_DOUBLE_EQ(red.longitude[0], full.longitude[7]);
  EXPECT_DOUBLE_EQ(red.longitude[1], full.longitude[8]);
  EXPECT_DOUBLE_EQ(red.longitude[2], full.longitude[9]);
  EXPECT_DOUBLE_EQ(red.longitude[3], full.longitude[0]);
  EXPECT_DOUBLE_EQ(red.longitude[4], full.longitude[1]);
  EXPECT_DOUBLE_EQ(red.longitude[5], full.longitude[2]);

  // Check partition_map slicing for wrapped local grid
  for (int y_loc = 0; y_loc < red.ny_local; ++y_loc) {
    for (int x_loc = 0; x_loc < red.nx_local; ++x_loc) {
      int gx = (red.global_x_min + x_loc) % nx;
      int g_idx = y_loc * nx + gx;
      int l_idx = y_loc * red.nx_local + x_loc;
      EXPECT_EQ(red.partition_map[l_idx], partition_map[g_idx]);
    }
  }
}

TEST(UQOneDomainTest, ExtractOneDomainInvalidPartitionOrGrid) {
  FullGrid full;
  full.nx = 5;
  full.ny = 5;
  full.land_sea_map.assign(25, 1);

  std::vector<int> partition_map(25, 1);

  // Non-existent partition ID
  ReducedGrid red_invalid = extractOneDomain(full, partition_map, 99, 1);
  EXPECT_EQ(red_invalid.nx_local, 0);
  EXPECT_EQ(red_invalid.ny_local, 0);
  EXPECT_TRUE(red_invalid.land_sea_map.empty());

  // Mismatched partition_map size
  std::vector<int> short_map(10, 1);
  ReducedGrid red_short = extractOneDomain(full, short_map, 1, 1);
  EXPECT_EQ(red_short.nx_local, 0);
  EXPECT_EQ(red_short.ny_local, 0);

  // Invalid nx or ny
  FullGrid bad_full = full;
  bad_full.nx = -1;
  ReducedGrid red_bad_dim = extractOneDomain(bad_full, partition_map, 1, 1);
  EXPECT_EQ(red_bad_dim.nx_local, 0);
}
