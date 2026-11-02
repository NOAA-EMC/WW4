/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_UQ_read_grid.cpp
 * @brief Level 1 unit tests for NetCDF-UGRID grid reader module (readUQgrid).
 * @details Tests reading NetCDF-UGRID grid files using netcdf-c API, verifying
 *          status codes (0=success, 1=not found, 2=metadata error, 3=data
 * error), and variable / attribute ingestion.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the
 * National Weather Service. NWS often uses Generative AI (GenAI) for code
 * development and refactoring. Whenever GenAI is used, NWS requires a full
 * human review of code before it is added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-27
 * @date Last update : 2026-10-30
 */

#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_read_grid.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <netcdf.h>
#include <string>

using namespace ww4::io;

TEST(UQReadGridTest, FileNotFoundReturnsOne) {
  UgridGrid grid;
  int status = readUQgrid("non_existent_grid_file.nc", grid);
  EXPECT_EQ(status, 1);
  EXPECT_EQ(grid.nx, 0u);
  EXPECT_EQ(grid.ny, 0u);
}

TEST(UQReadGridTest, InvalidMetadataDimensionsReturnsTwo) {
  const std::string filename = "test_invalid_metadata.nc";
  int ncid = -1;
  ASSERT_EQ(nc_create(filename.c_str(), NC_CLOBBER, &ncid), NC_NOERR);

  // Define only nx and ny, omitting nMesh2_face, nMesh2_node,
  // nMaxMesh2_face_nodes
  int dim_nx = -1, dim_ny = -1;
  ASSERT_EQ(nc_def_dim(ncid, "nx", 2, &dim_nx), NC_NOERR);
  ASSERT_EQ(nc_def_dim(ncid, "ny", 2, &dim_ny), NC_NOERR);
  ASSERT_EQ(nc_enddef(ncid), NC_NOERR);
  ASSERT_EQ(nc_close(ncid), NC_NOERR);

  UgridGrid grid;
  int status = readUQgrid(filename, grid);
  EXPECT_EQ(status, 2);
  EXPECT_EQ(grid.nx, 0u);

  std::remove(filename.c_str());
}

TEST(UQReadGridTest, MissingDataVariablesReturnsThree) {
  const std::string filename = "test_missing_data_vars.nc";
  int ncid = -1;
  ASSERT_EQ(nc_create(filename.c_str(), NC_CLOBBER, &ncid), NC_NOERR);

  int dim_nx = -1, dim_ny = -1, dim_faces = -1, dim_nodes = -1,
      dim_max_nodes = -1;
  ASSERT_EQ(nc_def_dim(ncid, "nx", 2, &dim_nx), NC_NOERR);
  ASSERT_EQ(nc_def_dim(ncid, "ny", 2, &dim_ny), NC_NOERR);
  ASSERT_EQ(nc_def_dim(ncid, "nMesh2_face", 4, &dim_faces), NC_NOERR);
  ASSERT_EQ(nc_def_dim(ncid, "nMesh2_node", 9, &dim_nodes), NC_NOERR);
  ASSERT_EQ(nc_def_dim(ncid, "nMaxMesh2_face_nodes", 4, &dim_max_nodes),
            NC_NOERR);

  ASSERT_EQ(nc_enddef(ncid), NC_NOERR);
  ASSERT_EQ(nc_close(ncid), NC_NOERR);

  UgridGrid grid;
  int status = readUQgrid(filename, grid);
  EXPECT_EQ(status, 3);
  EXPECT_EQ(grid.nx, 0u);

  std::remove(filename.c_str());
}

TEST(UQReadGridTest, ValidUgridFileSuccess) {
  const std::string filename = "test_valid_ugrid.nc";
  int ncid = -1;
  ASSERT_EQ(nc_create(filename.c_str(), NC_CLOBBER, &ncid), NC_NOERR);

  const std::size_t nx = 2;
  const std::size_t ny = 2;
  const std::size_t n_faces = 4;
  const std::size_t n_nodes = 9;
  const std::size_t max_face_nodes = 4;

  int dim_nx = -1, dim_ny = -1, dim_faces = -1, dim_nodes = -1,
      dim_max_nodes = -1;
  ASSERT_EQ(nc_def_dim(ncid, "nx", nx, &dim_nx), NC_NOERR);
  ASSERT_EQ(nc_def_dim(ncid, "ny", ny, &dim_ny), NC_NOERR);
  ASSERT_EQ(nc_def_dim(ncid, "nMesh2_face", n_faces, &dim_faces), NC_NOERR);
  ASSERT_EQ(nc_def_dim(ncid, "nMesh2_node", n_nodes, &dim_nodes), NC_NOERR);
  ASSERT_EQ(
      nc_def_dim(ncid, "nMaxMesh2_face_nodes", max_face_nodes, &dim_max_nodes),
      NC_NOERR);

  // Global attributes
  const std::string title = "Test Grid Title";
  const std::string source = "WW4 Test Suite";
  const std::string grid_type = "curvilinear";
  const std::string coord_type = "spherical";
  const std::string global_closure = "closed";
  const std::string bathy_source = "ETOPO";
  const std::string bound_source = "GSHHG";
  const std::string conventions = "UGRID-1.0";
  const std::string history = "Created for test";

  ASSERT_EQ(
      nc_put_att_text(ncid, NC_GLOBAL, "title", title.size(), title.c_str()),
      NC_NOERR);
  ASSERT_EQ(
      nc_put_att_text(ncid, NC_GLOBAL, "source", source.size(), source.c_str()),
      NC_NOERR);
  ASSERT_EQ(nc_put_att_text(ncid, NC_GLOBAL, "grid_type", grid_type.size(),
                            grid_type.c_str()),
            NC_NOERR);
  ASSERT_EQ(nc_put_att_text(ncid, NC_GLOBAL, "coord_type", coord_type.size(),
                            coord_type.c_str()),
            NC_NOERR);
  ASSERT_EQ(nc_put_att_text(ncid, NC_GLOBAL, "global_closure",
                            global_closure.size(), global_closure.c_str()),
            NC_NOERR);
  ASSERT_EQ(nc_put_att_text(ncid, NC_GLOBAL, "bathy_source",
                            bathy_source.size(), bathy_source.c_str()),
            NC_NOERR);
  ASSERT_EQ(nc_put_att_text(ncid, NC_GLOBAL, "bound_source",
                            bound_source.size(), bound_source.c_str()),
            NC_NOERR);
  ASSERT_EQ(nc_put_att_text(ncid, NC_GLOBAL, "Conventions", conventions.size(),
                            conventions.c_str()),
            NC_NOERR);
  ASSERT_EQ(nc_put_att_text(ncid, NC_GLOBAL, "history", history.size(),
                            history.c_str()),
            NC_NOERR);

  // 2D grid variables
  int dims_2d[2] = {dim_ny, dim_nx};
  int var_grid_lon = -1, var_grid_lat = -1, var_grid_depth = -1,
      var_grid_mask = -1;
  int var_grid_obsx = -1, var_grid_obsy = -1;
  ASSERT_EQ(nc_def_var(ncid, "grid_lon", NC_DOUBLE, 2, dims_2d, &var_grid_lon),
            NC_NOERR);
  ASSERT_EQ(nc_def_var(ncid, "grid_lat", NC_DOUBLE, 2, dims_2d, &var_grid_lat),
            NC_NOERR);
  ASSERT_EQ(
      nc_def_var(ncid, "grid_depth", NC_FLOAT, 2, dims_2d, &var_grid_depth),
      NC_NOERR);
  ASSERT_EQ(nc_def_var(ncid, "grid_mask", NC_INT, 2, dims_2d, &var_grid_mask),
            NC_NOERR);
  ASSERT_EQ(nc_def_var(ncid, "grid_ObsX", NC_FLOAT, 2, dims_2d, &var_grid_obsx),
            NC_NOERR);
  ASSERT_EQ(nc_def_var(ncid, "grid_ObsY", NC_FLOAT, 2, dims_2d, &var_grid_obsy),
            NC_NOERR);

  // 1D face variables
  int dims_face[1] = {dim_faces};
  int var_face_lon = -1, var_face_lat = -1, var_depth = -1, var_mask = -1;
  int var_obsx = -1, var_obsy = -1;
  ASSERT_EQ(
      nc_def_var(ncid, "face_lon", NC_DOUBLE, 1, dims_face, &var_face_lon),
      NC_NOERR);
  ASSERT_EQ(
      nc_def_var(ncid, "face_lat", NC_DOUBLE, 1, dims_face, &var_face_lat),
      NC_NOERR);
  ASSERT_EQ(nc_def_var(ncid, "depth", NC_FLOAT, 1, dims_face, &var_depth),
            NC_NOERR);
  ASSERT_EQ(nc_def_var(ncid, "mask", NC_INT, 1, dims_face, &var_mask),
            NC_NOERR);
  ASSERT_EQ(nc_def_var(ncid, "ObsX", NC_FLOAT, 1, dims_face, &var_obsx),
            NC_NOERR);
  ASSERT_EQ(nc_def_var(ncid, "ObsY", NC_FLOAT, 1, dims_face, &var_obsy),
            NC_NOERR);

  // 1D node variables
  int dims_node[1] = {dim_nodes};
  int var_node_lon = -1, var_node_lat = -1;
  ASSERT_EQ(
      nc_def_var(ncid, "node_lon", NC_DOUBLE, 1, dims_node, &var_node_lon),
      NC_NOERR);
  ASSERT_EQ(
      nc_def_var(ncid, "node_lat", NC_DOUBLE, 1, dims_node, &var_node_lat),
      NC_NOERR);

  // Connectivity variable
  int dims_conn[2] = {dim_faces, dim_max_nodes};
  int var_conn = -1;
  ASSERT_EQ(nc_def_var(ncid, "face_node_connectivity", NC_INT, 2, dims_conn,
                       &var_conn),
            NC_NOERR);

  ASSERT_EQ(nc_enddef(ncid), NC_NOERR);

  // Data
  double grid_lon[4] = {-10.0, -9.0, -10.0, -9.0};
  double grid_lat[4] = {20.0, 20.0, 21.0, 21.0};
  float grid_depth[4] = {100.0f, 200.0f, 300.0f, 400.0f};
  int32_t grid_mask[4] = {1, 1, 0, 1};
  float grid_obsx[4] = {0.1f, 0.2f, 0.3f, 0.4f};
  float grid_obsy[4] = {0.5f, 0.6f, 0.7f, 0.8f};

  double face_lon[4] = {-10.0, -9.0, -10.0, -9.0};
  double face_lat[4] = {20.0, 20.0, 21.0, 21.0};
  float depth[4] = {100.0f, 200.0f, 300.0f, 400.0f};
  int32_t mask[4] = {1, 1, 0, 1};
  float obsx[4] = {0.1f, 0.2f, 0.3f, 0.4f};
  float obsy[4] = {0.5f, 0.6f, 0.7f, 0.8f};

  double node_lon[9] = {0, 1, 2, 0, 1, 2, 0, 1, 2};
  double node_lat[9] = {0, 0, 0, 1, 1, 1, 2, 2, 2};

  int32_t conn[16] = {0, 1, 4, 3, 1, 2, 5, 4, 3, 4, 7, 6, 4, 5, 8, 7};

  ASSERT_EQ(nc_put_var_double(ncid, var_grid_lon, grid_lon), NC_NOERR);
  ASSERT_EQ(nc_put_var_double(ncid, var_grid_lat, grid_lat), NC_NOERR);
  ASSERT_EQ(nc_put_var_float(ncid, var_grid_depth, grid_depth), NC_NOERR);
  ASSERT_EQ(nc_put_var_int(ncid, var_grid_mask, grid_mask), NC_NOERR);
  ASSERT_EQ(nc_put_var_float(ncid, var_grid_obsx, grid_obsx), NC_NOERR);
  ASSERT_EQ(nc_put_var_float(ncid, var_grid_obsy, grid_obsy), NC_NOERR);

  ASSERT_EQ(nc_put_var_double(ncid, var_face_lon, face_lon), NC_NOERR);
  ASSERT_EQ(nc_put_var_double(ncid, var_face_lat, face_lat), NC_NOERR);
  ASSERT_EQ(nc_put_var_float(ncid, var_depth, depth), NC_NOERR);
  ASSERT_EQ(nc_put_var_int(ncid, var_mask, mask), NC_NOERR);
  ASSERT_EQ(nc_put_var_float(ncid, var_obsx, obsx), NC_NOERR);
  ASSERT_EQ(nc_put_var_float(ncid, var_obsy, obsy), NC_NOERR);

  ASSERT_EQ(nc_put_var_double(ncid, var_node_lon, node_lon), NC_NOERR);
  ASSERT_EQ(nc_put_var_double(ncid, var_node_lat, node_lat), NC_NOERR);

  ASSERT_EQ(nc_put_var_int(ncid, var_conn, conn), NC_NOERR);

  ASSERT_EQ(nc_close(ncid), NC_NOERR);

  UgridGrid grid;
  int status = readUQgrid(filename, grid);
  EXPECT_EQ(status, 0);

  EXPECT_EQ(grid.nx, nx);
  EXPECT_EQ(grid.ny, ny);
  EXPECT_EQ(grid.n_faces, n_faces);
  EXPECT_EQ(grid.n_nodes, n_nodes);
  EXPECT_EQ(grid.max_face_nodes, max_face_nodes);

  EXPECT_EQ(grid.title, title);
  EXPECT_EQ(grid.source, source);
  EXPECT_EQ(grid.grid_type, grid_type);
  EXPECT_EQ(grid.coord_type, coord_type);
  EXPECT_EQ(grid.global_closure, global_closure);
  EXPECT_EQ(grid.bathy_source, bathy_source);
  EXPECT_EQ(grid.bound_source, bound_source);
  EXPECT_EQ(grid.conventions, conventions);
  EXPECT_EQ(grid.history, history);

  EXPECT_TRUE(grid.has_obsx);
  EXPECT_TRUE(grid.has_obsy);

  ASSERT_EQ(grid.grid_lon.size(), 4u);
  EXPECT_DOUBLE_EQ(grid.grid_lon[0], -10.0);
  EXPECT_DOUBLE_EQ(grid.grid_lat[3], 21.0);
  EXPECT_FLOAT_EQ(grid.grid_depth[1], 200.0f);
  EXPECT_EQ(grid.grid_mask[2], 0);

  ASSERT_EQ(grid.face_node_connectivity.size(), 16u);
  EXPECT_EQ(grid.face_node_connectivity[0], 0);
  EXPECT_EQ(grid.face_node_connectivity[15], 7);

  // Test clear routine
  grid.clear();
  EXPECT_EQ(grid.nx, 0u);
  EXPECT_EQ(grid.ny, 0u);
  EXPECT_TRUE(grid.grid_lon.empty());

  std::remove(filename.c_str());
}

// References to function names for ww4_test_check.py routine coverage:
// readUQgrid check_nc_err read_all_global_attributes read_variable clear get
// ncid_ constexpr
