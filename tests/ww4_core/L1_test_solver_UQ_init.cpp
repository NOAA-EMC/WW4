#include "ww4_core/solver_UQ/utils_UQ_IO/test_ugrid_helper.h"
/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_solver_UQ_init.cpp
 * @brief Unit tests for the UQ solver initialization module.
 * @details Verifies solver_UQ_init execution.
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
 * @date Initial, 2026-10-23
 * @date Last update : 2026-11-02
 */

#include "ww4_core/solver_UQ/solver_UQ_init.h"
#include "ww4_core/w4core_init.h"
#include "ww4_utils/ww4_logfile.h"
#include <fstream>
#include <gtest/gtest.h>
#include <netcdf.h>
#include <sstream>

namespace ww4_core {

TEST(SolverUQInitTest, BasicExecutionAndOutput) {
  resetInternalState();
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "my_uq_grid.nc";
  getMutableRunConfig().printGridMeta = true;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  { ww4::test_helpers::createMinimalUgridFile("my_uq_grid.nc", 10, 10); }

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(output.find("Processing the grid:"), std::string::npos);
  EXPECT_NE(output.find("     Grid file: my_uq_grid.nc"), std::string::npos);
  EXPECT_NE(output.find("     Grid metadata:"), std::string::npos);
  EXPECT_NE(output.find("Grid dimensions       : 10 x 10"), std::string::npos);

  std::remove("my_uq_grid.nc");
  resetInternalState();
}

TEST(SolverUQInitTest, OutputWithoutMetadataWhenDisabled) {
  resetInternalState();
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "my_uq_grid_nometa.nc";
  getMutableRunConfig().printGridMeta = false;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  { ww4::test_helpers::createMinimalUgridFile("my_uq_grid_nometa.nc", 10, 10); }

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(output.find("Processing the grid:"), std::string::npos);
  EXPECT_NE(output.find("     Grid file: my_uq_grid_nometa.nc"),
            std::string::npos);
  EXPECT_EQ(output.find("     Grid metadata:"), std::string::npos);

  std::remove("my_uq_grid_nometa.nc");
  resetInternalState();
}

TEST(SolverUQInitTest, AbortsWhenFileNotFound) {
  resetInternalState();
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "non_existent_grid_file.nc";
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  EXPECT_DEATH(solver_UQ_init(std::cerr),
               "Grid file does not exist or failed opening");
  resetInternalState();
}

TEST(SolverUQInitTest, AbortsWhenMetadataInvalid) {
  resetInternalState();
  const std::string filename = "test_bad_metadata.nc";
  int ncid = -1;
  ASSERT_EQ(nc_create(filename.c_str(), NC_CLOBBER, &ncid), NC_NOERR);
  int dim_nx = -1, dim_ny = -1;
  ASSERT_EQ(nc_def_dim(ncid, "nx", 2, &dim_nx), NC_NOERR);
  ASSERT_EQ(nc_def_dim(ncid, "ny", 2, &dim_ny), NC_NOERR);
  ASSERT_EQ(nc_enddef(ncid), NC_NOERR);
  ASSERT_EQ(nc_close(ncid), NC_NOERR);

  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = filename;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  EXPECT_DEATH(solver_UQ_init(std::cerr),
               "Failed reading metadata dimensions from file");

  std::remove(filename.c_str());
  resetInternalState();
}

TEST(SolverUQInitTest, AbortsWhenDataVariablesMissing) {
  resetInternalState();
  const std::string filename = "test_bad_data_vars.nc";
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

  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = filename;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  EXPECT_DEATH(solver_UQ_init(std::cerr),
               "Failed reading grid dataset arrays from file");

  std::remove(filename.c_str());
  resetInternalState();
}

} // namespace ww4_core
