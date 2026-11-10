/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_UQ_write_partition.cpp
 * @brief Level 1 unit tests for NetCDF partition map writer tools.
 * @details Tests opening, incremental writing of multiple maps, and error
 *          handling.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the
 * National Weather Service. NWS often uses Generative AI (GenAI) for code
 * development and refactoring. Whenever GenAI is used, NWS requires a full
 * human review of code before it is added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-11-03
 * @date Last update : 2026-11-05
 */

#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_read_partition.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_write_partition.h"
#include <cstdio>
#include <gtest/gtest.h>

using namespace ww4::io;

TEST(UQWritePartitionTest, OneShotWrite) {
  const std::string filename = "test_write_oneshot_part.nc";
  UgridGrid grid;
  grid.nx = 3;
  grid.ny = 2;
  grid.title = "Test Grid Partition";
  grid.source = "WW4 Partition Test";
  grid.grid_type = "curvilinear";
  grid.coord_type = "spherical";
  grid.global_closure = "closed";
  grid.bathy_source = "ETOPO";
  grid.bound_source = "GSHHG";
  grid.conventions = "UGRID-1.0";
  grid.history = "2026-11-03 created";

  const int num_partitions = 2;
  const std::vector<int32_t> partition_map = {1, 1, 2, 1, 2, 2};

  int status =
      writeUQpartitionMapNetCDF(filename, grid, num_partitions, partition_map);
  ASSERT_EQ(status, 0);

  PartitionMapData read_data;
  status = readUQpartitionMapNetCDF(filename, read_data);
  ASSERT_EQ(status, 0);
  EXPECT_EQ(read_data.nx, 3u);
  EXPECT_EQ(read_data.ny, 2u);
  EXPECT_EQ(read_data.n_faces, 6u);
  EXPECT_EQ(read_data.n_nodes, 12u);
  EXPECT_EQ(read_data.max_face_nodes, 4u);
  EXPECT_EQ(read_data.partition_map, partition_map);

  std::remove(filename.c_str());
}

TEST(UQWritePartitionTest, IncrementalMultipleMaps) {
  const std::string filename = "test_write_evolution_part.nc";
  UgridGrid grid;
  grid.nx = 4;
  grid.ny = 2;
  grid.title = "Map Evolution Test";
  grid.global_attrs["evolution_note"] = "Documenting iterative balance stages";

  const int num_partitions = 3;
  int ncid = -1;
  int open_status =
      openUQpartitionMapFile(filename, grid, num_partitions, ncid);
  ASSERT_EQ(open_status, 0);
  ASSERT_GE(ncid, 0);

  std::vector<int32_t> map_stage0 = {1, 1, 2, 2, 1, 3, 3, 2};
  std::vector<int32_t> map_stage1 = {1, 1, 2, 2, 1, 1, 3, 3};
  std::vector<int32_t> map_stage2 = {1, 1, 1, 2, 2, 3, 3, 3};

  std::size_t map_idx = 0;
  EXPECT_EQ(writeSingleUQpartitionMap(ncid, map_stage0, map_idx), 0);
  EXPECT_EQ(map_idx, 1u);

  EXPECT_EQ(writeSingleUQpartitionMap(ncid, map_stage1, map_idx), 0);
  EXPECT_EQ(map_idx, 2u);

  EXPECT_EQ(writeSingleUQpartitionMap(ncid, map_stage2, map_idx), 0);
  EXPECT_EQ(map_idx, 3u);

  EXPECT_EQ(closeUQpartitionMapFile(ncid), 0);

  PartitionMapData read_data;
  int read_status = readUQpartitionMapNetCDF(filename, read_data);
  ASSERT_EQ(read_status, 0);
  EXPECT_EQ(read_data.partition_map, map_stage2);

  std::remove(filename.c_str());
}

TEST(UQWritePartitionTest, ErrorHandlingInWriter) {
  UgridGrid grid;
  grid.nx = 0;
  grid.ny = 0;
  int ncid = -1;
  EXPECT_EQ(openUQpartitionMapFile("test_err.nc", grid, 2, ncid), 2);

  grid.nx = 2;
  grid.ny = 2;
  EXPECT_EQ(openUQpartitionMapFile("test_err.nc", grid, 0, ncid), 2);

  std::vector<int32_t> pmap = {1, 2, 1, 2};
  std::size_t idx = 0;
  EXPECT_EQ(writeSingleUQpartitionMap(-1, pmap, idx), 3);
  EXPECT_EQ(closeUQpartitionMapFile(-1), 0);
}

// References to function names for ww4_test_check.py routine coverage:
// openPartitionMapFile writeSinglePartitionMap closePartitionMapFile
// writePartitionMapNetCDF check_nc_part_err openUQpartitionMapFile
// writeSingleUQpartitionMap closeUQpartitionMapFile writeUQpartitionMapNetCDF
