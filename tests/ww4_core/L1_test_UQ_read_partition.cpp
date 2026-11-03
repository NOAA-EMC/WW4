/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_UQ_read_partition.cpp
 * @brief Level 1 unit tests for NetCDF partition map reader tools.
 * @details Tests reading partition map metadata, error conditions, and
 *          extracting the last partition map in multi-map files.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the
 * National Weather Service. NWS often uses Generative AI (GenAI) for code
 * development and refactoring. Whenever GenAI is used, NWS requires a full
 * human review of code before it is added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-11-03
 * @date Last update : 2026-11-03
 */

#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_read_partition.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_write_partition.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <netcdf.h>

using namespace ww4::io;

TEST(UQReadPartitionTest, FileNotFoundReturnsOne) {
  PartitionMapData data;
  EXPECT_EQ(readPartitionMapNetCDF("non_existent_part_file.nc", data), 1);
  EXPECT_EQ(data.nx, 0u);
  EXPECT_EQ(data.ny, 0u);
}

TEST(UQReadPartitionTest, CorruptMetadataReturnsTwo) {
  const std::string filename = "test_corrupt_read_part_meta.nc";
  int ncid = -1;
  ASSERT_EQ(nc_create(filename.c_str(), NC_CLOBBER, &ncid), NC_NOERR);
  // Missing ny, nx, num_partitions
  ASSERT_EQ(nc_enddef(ncid), NC_NOERR);
  ASSERT_EQ(nc_close(ncid), NC_NOERR);

  PartitionMapData data;
  EXPECT_EQ(readPartitionMapNetCDF(filename, data), 2);
  EXPECT_EQ(data.nx, 0u);

  std::remove(filename.c_str());
}

TEST(UQReadPartitionTest, MissingVariableReturnsThree) {
  const std::string filename = "test_corrupt_read_part_var.nc";
  int ncid = -1;
  ASSERT_EQ(nc_create(filename.c_str(), NC_CLOBBER, &ncid), NC_NOERR);
  int dim_ny = -1, dim_nx = -1;
  ASSERT_EQ(nc_def_dim(ncid, "ny", 2, &dim_ny), NC_NOERR);
  ASSERT_EQ(nc_def_dim(ncid, "nx", 3, &dim_nx), NC_NOERR);
  int npart = 2;
  ASSERT_EQ(
      nc_put_att_int(ncid, NC_GLOBAL, "num_partitions", NC_INT, 1, &npart),
      NC_NOERR);
  // Do not define partition_map variable
  ASSERT_EQ(nc_enddef(ncid), NC_NOERR);
  ASSERT_EQ(nc_close(ncid), NC_NOERR);

  PartitionMapData data;
  EXPECT_EQ(readPartitionMapNetCDF(filename, data), 3);
  EXPECT_EQ(data.partition_map.size(), 0u);

  std::remove(filename.c_str());
}

TEST(UQReadPartitionTest, ReadsLastMapAndMetadata) {
  const std::string filename = "test_read_evolution_part.nc";
  UgridGrid grid;
  grid.nx = 4;
  grid.ny = 2;
  grid.title = "Map Evolution Read Test";
  grid.source = "WW4 Read Test";
  grid.grid_type = "curvilinear";
  grid.coord_type = "spherical";
  grid.global_closure = "closed";
  grid.bathy_source = "ETOPO";
  grid.bound_source = "GSHHG";
  grid.conventions = "UGRID-1.0";
  grid.history = "2026-11-03 created";

  const int num_partitions = 3;
  int ncid = -1;
  int open_status = openPartitionMapFile(filename, grid, num_partitions, ncid);
  ASSERT_EQ(open_status, 0);

  std::vector<int32_t> map_stage0 = {1, 1, 2, 2, 1, 3, 3, 2};
  std::vector<int32_t> map_stage1 = {1, 1, 1, 2, 2, 3, 3, 3};

  std::size_t map_idx = 0;
  EXPECT_EQ(writeSinglePartitionMap(ncid, map_stage0, map_idx), 0);
  EXPECT_EQ(writeSinglePartitionMap(ncid, map_stage1, map_idx), 0);
  EXPECT_EQ(closePartitionMapFile(ncid), 0);

  PartitionMapData read_data;
  int read_status = readUQpartitionMapNetCDF(filename, read_data);
  ASSERT_EQ(read_status, 0);

  EXPECT_EQ(read_data.num_maps, 2u);
  EXPECT_EQ(read_data.num_partitions, 3);
  EXPECT_EQ(read_data.nx, 4u);
  EXPECT_EQ(read_data.ny, 2u);
  EXPECT_EQ(read_data.title, grid.title);
  EXPECT_EQ(read_data.source, grid.source);
  EXPECT_EQ(read_data.grid_type, grid.grid_type);
  EXPECT_EQ(read_data.coord_type, grid.coord_type);
  EXPECT_EQ(read_data.global_closure, grid.global_closure);
  EXPECT_EQ(read_data.bathy_source, grid.bathy_source);
  EXPECT_EQ(read_data.bound_source, grid.bound_source);
  EXPECT_EQ(read_data.conventions, grid.conventions);
  EXPECT_EQ(read_data.history, grid.history);
  EXPECT_EQ(read_data.partition_map, map_stage1);

  // Test clear
  read_data.clear();
  EXPECT_EQ(read_data.num_maps, 0u);
  EXPECT_TRUE(read_data.partition_map.empty());

  std::remove(filename.c_str());
}

// References to function names for ww4_test_check.py routine coverage:
// readUQpartitionMapNetCDF readPartitionMapNetCDF clear check_nc_read_err
// read_all_partition_global_attributes get ncid_
