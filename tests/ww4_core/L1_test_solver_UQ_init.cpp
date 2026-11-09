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
 * @date Last update : 2026-11-06
 */

#include "ww4_core/solver_UQ/solver_UQ_init.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_read_partition.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_write_partition.h"
#include "ww4_core/w4core_init.h"
#include "ww4_utils/ww4_logfile.h"
#include <filesystem>
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

  ww4::test_helpers::createMinimalUgridFile("my_uq_grid.nc", 10, 10);

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(output.find("Processing the grid:"), std::string::npos);
  EXPECT_NE(output.find("     Grid file                : my_uq_grid.nc"),
            std::string::npos);
  EXPECT_NE(output.find("     Grid metadata:"), std::string::npos);
  EXPECT_NE(output.find("Grid dimensions       : NX = 10, NY = 10"),
            std::string::npos);

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

  ww4::test_helpers::createMinimalUgridFile("my_uq_grid_nometa.nc", 10, 10);

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(output.find("Processing the grid:"), std::string::npos);
  EXPECT_NE(output.find("     Grid file                : my_uq_grid_nometa.nc"),
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

TEST(SolverUQInitTest, PartitionFileNotFoundLogsErrorAndComputesPartitions) {
  resetInternalState();
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "my_uq_grid_nopart.nc";
  getMutableRunConfig().numRanksCompute = 5;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  ww4::test_helpers::createMinimalUgridFile("my_uq_grid_nopart.nc", 10, 10);

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(
      output.find("Partition file           : my_uq_grid_nopart.partitions.nc"),
      std::string::npos);
  EXPECT_NE(output.find("Partition read error     : Partition file does not "
                        "exist or failed opening"),
            std::string::npos);
  // Ensure the file name is NOT reproduced in the partition read error line
  EXPECT_EQ(output.find("Partition file does not exist or failed opening: "
                        "my_uq_grid_nopart.partitions.nc"),
            std::string::npos);
  // Ensure message "Creating the partition map for NNN partitions" is written
  EXPECT_NE(output.find("Creating the partition map for 5 partitions"),
            std::string::npos);
  EXPECT_NE(output.find("Writing partition file with 1 partition map"),
            std::string::npos);

  // Check that the new partition file was written
  EXPECT_TRUE(std::filesystem::exists("my_uq_grid_nopart.partitions.nc"));

  std::remove("my_uq_grid_nopart.nc");
  std::remove("my_uq_grid_nopart.partitions.nc");
  resetInternalState();
}

TEST(SolverUQInitTest, PartitionFileExistsAndLoadsSuccessfully) {
  resetInternalState();
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "my_uq_grid_withpart.nc";
  getMutableRunConfig().numRanksCompute = 1;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  {
    ww4::test_helpers::createMinimalUgridFile("my_uq_grid_withpart.nc", 10, 10);
  }

  ww4::io::UgridGrid ugrid;
  ASSERT_EQ(ww4::io::readUQgrid("my_uq_grid_withpart.nc", ugrid), 0);
  std::vector<int32_t> partMap(10 * 10, 1);
  ASSERT_EQ(ww4::io::writeUQpartitionMapNetCDF(
                "my_uq_grid_withpart.partitions.nc", ugrid, 1, partMap),
            0);

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(output.find(
                "Partition file           : my_uq_grid_withpart.partitions.nc"),
            std::string::npos);
  EXPECT_EQ(output.find("Partition read error"), std::string::npos);
  EXPECT_EQ(output.find("Creating the partition map"), std::string::npos);

  std::remove("my_uq_grid_withpart.nc");
  std::remove("my_uq_grid_withpart.partitions.nc");
  resetInternalState();
}

TEST(SolverUQInitTest,
     PartitionCountMismatchLogsErrorAndRegeneratesPartitions) {
  resetInternalState();
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "my_uq_grid_mismatch.nc";
  getMutableRunConfig().numRanksCompute = 4;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  {
    ww4::test_helpers::createMinimalUgridFile("my_uq_grid_mismatch.nc", 10, 10);
  }

  ww4::io::UgridGrid ugrid;
  ASSERT_EQ(ww4::io::readUQgrid("my_uq_grid_mismatch.nc", ugrid), 0);
  std::vector<int32_t> partMap(10 * 10, 1);
  // File created with 2 partitions, but user request is 4
  ASSERT_EQ(ww4::io::writeUQpartitionMapNetCDF(
                "my_uq_grid_mismatch.partitions.nc", ugrid, 2, partMap),
            0);

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(output.find("Partition read error     : Number of partitions in "
                        "file does not match user request"),
            std::string::npos);
  // Ensure the file name is NOT in the error message line
  EXPECT_EQ(output.find("Partition read error     : Number of partitions in "
                        "file does not match user request: "
                        "my_uq_grid_mismatch.partitions.nc"),
            std::string::npos);
  EXPECT_NE(output.find("Creating the partition map for 4 partitions"),
            std::string::npos);
  EXPECT_NE(output.find("Writing partition file with 1 partition map"),
            std::string::npos);

  // Check that the new partition file was updated with 4 partitions
  ww4::io::PartitionMapData updatedPart;
  ASSERT_EQ(ww4::io::readUQpartitionMapNetCDF(
                "my_uq_grid_mismatch.partitions.nc", updatedPart),
            0);
  EXPECT_EQ(updatedPart.num_partitions, 4);

  std::remove("my_uq_grid_mismatch.nc");
  std::remove("my_uq_grid_mismatch.partitions.nc");
  resetInternalState();
}

TEST(SolverUQInitTest, PartitionMetadataMismatchLogsErrorAndRegenerates) {
  resetInternalState();
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "my_uq_grid_metadiff.nc";
  getMutableRunConfig().numRanksCompute = 1;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  ww4::test_helpers::createMinimalUgridFile("my_uq_grid_metadiff.nc", 10, 10);

  // Read grid, then modify an attribute before writing partition file
  ww4::io::UgridGrid ugrid;
  ASSERT_EQ(ww4::io::readUQgrid("my_uq_grid_metadiff.nc", ugrid), 0);
  ugrid.title = "Different Title";

  std::vector<int32_t> partMap(10 * 10, 1);
  ASSERT_EQ(ww4::io::writeUQpartitionMapNetCDF(
                "my_uq_grid_metadiff.partitions.nc", ugrid, 1, partMap),
            0);

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(output.find("Partition read error     : Metadata in partition "
                        "file does not match grid file"),
            std::string::npos);
  EXPECT_NE(output.find("Creating the partition map for 1 partitions"),
            std::string::npos);

  // Check that the partition file was updated with original grid title
  ww4::io::PartitionMapData updatedPart;
  ASSERT_EQ(ww4::io::readUQpartitionMapNetCDF(
                "my_uq_grid_metadiff.partitions.nc", updatedPart),
            0);
  EXPECT_EQ(updatedPart.title, "my_uq_grid_metadiff.nc");

  std::remove("my_uq_grid_metadiff.nc");
  std::remove("my_uq_grid_metadiff.partitions.nc");
  resetInternalState();
}

TEST(SolverUQInitTest, PartitionDimensionsMismatchLogsErrorAndRegenerates) {
  resetInternalState();
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "my_uq_grid_dimdiff.nc";
  getMutableRunConfig().numRanksCompute = 1;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  ww4::test_helpers::createMinimalUgridFile("my_uq_grid_dimdiff.nc", 10, 10);

  // Create partition file with 8x8 instead of 10x10
  ww4::io::UgridGrid ugrid8;
  ww4::test_helpers::createMinimalUgridFile("temp_8x8.nc", 8, 8);
  ASSERT_EQ(ww4::io::readUQgrid("temp_8x8.nc", ugrid8), 0);
  std::remove("temp_8x8.nc");

  // Keep title matching so title doesn't trigger it, but dimensions do
  ugrid8.title = "Minimal UGRID Grid";
  std::vector<int32_t> partMap8(8 * 8, 1);
  ASSERT_EQ(ww4::io::writeUQpartitionMapNetCDF(
                "my_uq_grid_dimdiff.partitions.nc", ugrid8, 1, partMap8),
            0);

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(output.find("Partition read error     : Metadata in partition "
                        "file does not match grid file"),
            std::string::npos);
  EXPECT_NE(output.find("Creating the partition map for 1 partitions"),
            std::string::npos);

  // Verify updated partition has 10x10
  ww4::io::PartitionMapData updatedPart;
  ASSERT_EQ(ww4::io::readUQpartitionMapNetCDF(
                "my_uq_grid_dimdiff.partitions.nc", updatedPart),
            0);
  EXPECT_EQ(updatedPart.nx, 10u);
  EXPECT_EQ(updatedPart.ny, 10u);

  std::remove("my_uq_grid_dimdiff.nc");
  std::remove("my_uq_grid_dimdiff.partitions.nc");
  resetInternalState();
}

TEST(SolverUQInitTest, PartitionFileCorruptedLogsErrorAndDoesNotAbort) {
  resetInternalState();
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "my_uq_grid_badpart.nc";
  getMutableRunConfig().numRanksCompute = 5;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  {
    ww4::test_helpers::createMinimalUgridFile("my_uq_grid_badpart.nc", 10, 10);
  }

  // Create corrupted partition file missing required dimensions/attributes
  int ncid = -1;
  ASSERT_EQ(nc_create("my_uq_grid_badpart.partitions.nc", NC_CLOBBER, &ncid),
            NC_NOERR);
  ASSERT_EQ(nc_close(ncid), NC_NOERR);

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(output.find(
                "Partition file           : my_uq_grid_badpart.partitions.nc"),
            std::string::npos);
  EXPECT_NE(output.find("Partition read error     : Failed reading metadata "
                        "dimensions from file"),
            std::string::npos);
  EXPECT_EQ(output.find("Failed reading metadata dimensions from file: "
                        "my_uq_grid_badpart.partitions.nc"),
            std::string::npos);
  EXPECT_NE(output.find("Creating the partition map for 5 partitions"),
            std::string::npos);

  std::remove("my_uq_grid_badpart.nc");
  std::remove("my_uq_grid_badpart.partitions.nc");
  resetInternalState();
}

TEST(SolverUQInitTest, IntermediateMapsWritesMultipleMaps) {
  resetInternalState();
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "my_uq_grid_interm.nc";
  getMutableRunConfig().numRanksCompute = 4;
  getMutableRunConfig().intermediateMaps = true;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  ww4::test_helpers::createMinimalUgridFile("my_uq_grid_interm.nc", 12, 12);

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(output.find("Creating the partition map for 4 partitions"),
            std::string::npos);
  // Check that "Writing partition file with" is in the output
  EXPECT_NE(output.find("Writing partition file with "), std::string::npos);

  // Read partition file to verify maps
  ww4::io::PartitionMapData readData;
  ASSERT_EQ(ww4::io::readUQpartitionMapNetCDF("my_uq_grid_interm.partitions.nc",
                                              readData),
            0);
  EXPECT_EQ(readData.num_partitions, 4);
  EXPECT_GE(readData.num_maps, 1u);

  std::remove("my_uq_grid_interm.nc");
  std::remove("my_uq_grid_interm.partitions.nc");
  resetInternalState();
}

TEST(SolverUQInitTest, NoHistogramFinalDomainSizes) {
  resetInternalState();
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "my_uq_grid_hist.nc";
  getMutableRunConfig().numRanksCompute = 2;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  ww4::test_helpers::createMinimalUgridFile("my_uq_grid_hist.nc", 8, 8);

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_EQ(output.find("Final domain sizes"), std::string::npos);
  EXPECT_EQ(output.find("points :"), std::string::npos);

  // Test that reading an existing partition file also does NOT output histogram
  ss.str("");
  ss.clear();
  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string existingOutput = ss.str();
  EXPECT_EQ(existingOutput.find("Final domain sizes"), std::string::npos);

  std::remove("my_uq_grid_hist.nc");
  std::remove("my_uq_grid_hist.partitions.nc");
  resetInternalState();
}

TEST(SolverUQInitTest, WriteLocalGridGeneratesLocalUgridFile) {
  resetInternalState();
  std::ostringstream ss;
  getMutableRunConfig().produceStdOut = true;
  getMutableRunConfig().gridFile = "my_local_test_ugrid.nc";
  getMutableRunConfig().numRanks = 3;
  getMutableRunConfig().numRanksCompute = 2;
  getMutableRunConfig().actualRank = 0;
  getMutableRunConfig().writeLocalGrid = true;
  getMutableRunConfig().screenOutputLevel = ww4_utils::ScreenOutputLevel::Full;

  ww4::test_helpers::createMinimalUgridFile("my_local_test_ugrid.nc", 8, 8);

  EXPECT_NO_THROW(solver_UQ_init(ss));
  std::string output = ss.str();
  EXPECT_NE(output.find("Local domain grid file   :"), std::string::npos);
  EXPECT_NE(output.find("my_local_test.part000.nc"), std::string::npos);

  // Read the generated local domain file and verify its contents
  ww4::io::UgridGrid localUgrid;
  int readStatus = ww4::io::readUQgrid("my_local_test.part000.nc", localUgrid);
  EXPECT_EQ(readStatus, 0);
  EXPECT_GT(localUgrid.nx, 0u);
  EXPECT_GT(localUgrid.ny, 0u);
  EXPECT_EQ(localUgrid.grid_mask.size(), localUgrid.nx * localUgrid.ny);

  // Verify that the local grid mask contains wet points (1 or 2)
  bool hasActive = false;
  for (int m : localUgrid.grid_mask) {
    if (m == 1 || m == 2) {
      hasActive = true;
      break;
    }
  }
  EXPECT_TRUE(hasActive);

  std::remove("my_local_test_ugrid.nc");
  std::remove("my_local_test.partitions.nc");
  std::remove("my_local_test.part000.nc");
  resetInternalState();
}

} // namespace ww4_core
