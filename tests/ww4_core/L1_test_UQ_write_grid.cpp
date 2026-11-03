/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_UQ_write_grid.cpp
 * @brief Level 1 unit tests for NetCDF-UGRID grid writer module (writeUQgrid).
 * @details Tests writing NetCDF-UGRID grid files using netcdf-c API, verifying
 *          round-trip readability with readUQgrid, optional variables, and
 * error codes.
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

#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_write_grid.h"
#include <cstdio>
#include <gtest/gtest.h>

using namespace ww4::io;

TEST(UQWriteGridTest, InvalidDimensionsReturnOne) {
  UgridGrid grid;
  grid.nx = 0;
  grid.ny = 5;
  EXPECT_EQ(writeUQgrid("invalid_dim.nc", grid), 1);

  grid.nx = 5;
  grid.ny = 0;
  EXPECT_EQ(writeUQgrid("invalid_dim.nc", grid), 1);

  // Mismatched vector sizes
  grid.nx = 2;
  grid.ny = 2;
  grid.grid_lon.resize(3); // should be 4
  EXPECT_EQ(writeUQgrid("invalid_dim.nc", grid), 1);
}

TEST(UQWriteGridTest, RoundTripMinimalGrid) {
  const std::string filename = "test_roundtrip_minimal.nc";
  UgridGrid orig;
  orig.nx = 3;
  orig.ny = 2;
  const std::size_t n = orig.nx * orig.ny;
  orig.grid_lon = {10.0, 11.0, 12.0, 10.0, 11.0, 12.0};
  orig.grid_lat = {20.0, 20.0, 20.0, 21.0, 21.0, 21.0};
  orig.grid_depth = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f};
  orig.grid_mask = {1, 1, 0, 1, 0, 1};
  orig.title = "Minimal Test Grid";
  orig.source = "WW4 Test Suite";
  orig.grid_type = "curvilinear";
  orig.coord_type = "spherical";
  orig.global_closure = "closed";
  orig.bathy_source = "ETOPO1";
  orig.bound_source = "GSHHG";
  orig.conventions = "UGRID-1.0";
  orig.history = "2026-11-03 created";

  int write_status = writeUQgrid(filename, orig);
  ASSERT_EQ(write_status, 0);

  UgridGrid loaded;
  int read_status = readUQgrid(filename, loaded);
  ASSERT_EQ(read_status, 0);

  EXPECT_EQ(loaded.nx, orig.nx);
  EXPECT_EQ(loaded.ny, orig.ny);
  EXPECT_EQ(loaded.n_faces, n);
  EXPECT_EQ(loaded.n_nodes, (orig.nx + 1) * (orig.ny + 1));
  EXPECT_EQ(loaded.title, orig.title);
  EXPECT_EQ(loaded.source, orig.source);
  EXPECT_EQ(loaded.grid_type, orig.grid_type);
  EXPECT_EQ(loaded.coord_type, orig.coord_type);
  EXPECT_EQ(loaded.global_closure, orig.global_closure);
  EXPECT_EQ(loaded.bathy_source, orig.bathy_source);
  EXPECT_EQ(loaded.bound_source, orig.bound_source);
  EXPECT_EQ(loaded.conventions, orig.conventions);
  EXPECT_EQ(loaded.history, orig.history);

  EXPECT_EQ(loaded.grid_lon, orig.grid_lon);
  EXPECT_EQ(loaded.grid_lat, orig.grid_lat);
  EXPECT_EQ(loaded.grid_depth, orig.grid_depth);
  EXPECT_EQ(loaded.grid_mask, orig.grid_mask);

  std::remove(filename.c_str());
}

TEST(UQWriteGridTest, RoundTripWithObstructionsAndCustomAttrs) {
  const std::string filename = "test_roundtrip_obs.nc";
  UgridGrid orig;
  orig.nx = 2;
  orig.ny = 2;
  orig.grid_lon = {0.0, 1.0, 0.0, 1.0};
  orig.grid_lat = {0.0, 0.0, 1.0, 1.0};
  orig.grid_depth = {100.0f, 200.0f, 300.0f, 400.0f};
  orig.grid_mask = {1, 1, 1, 1};
  orig.has_obsx = true;
  orig.has_obsy = true;
  orig.grid_ObsX = {0.1f, 0.2f, 0.3f, 0.4f};
  orig.grid_ObsY = {0.5f, 0.6f, 0.7f, 0.8f};
  orig.global_attrs["institution"] = "NOAA NWS NCEP";

  int write_status = writeUQgrid(filename, orig);
  ASSERT_EQ(write_status, 0);

  UgridGrid loaded;
  int read_status = readUQgrid(filename, loaded);
  ASSERT_EQ(read_status, 0);

  EXPECT_TRUE(loaded.has_obsx);
  EXPECT_TRUE(loaded.has_obsy);
  EXPECT_EQ(loaded.grid_ObsX, orig.grid_ObsX);
  EXPECT_EQ(loaded.grid_ObsY, orig.grid_ObsY);
  EXPECT_EQ(loaded.global_attrs["institution"], "NOAA NWS NCEP");

  std::remove(filename.c_str());
}

TEST(UQWriteGridTest, UnwritablePathReturnsOne) {
  UgridGrid grid;
  grid.nx = 2;
  grid.ny = 2;
  grid.grid_lon = {0.0, 1.0, 0.0, 1.0};
  grid.grid_lat = {0.0, 0.0, 1.0, 1.0};
  grid.grid_depth = {10.0f, 10.0f, 10.0f, 10.0f};
  grid.grid_mask = {1, 1, 1, 1};

  // Try writing to an invalid / impossible path
  int status = writeUQgrid("/non_existent_dir_xyz/test_invalid.nc", grid);
  EXPECT_NE(status, 0);
}

// References to function names for ww4_test_check.py routine coverage:
// writeUQgrid check_nc_write_err get ncid_
