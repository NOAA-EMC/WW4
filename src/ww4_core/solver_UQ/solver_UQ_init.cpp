/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file solver_UQ_init.cpp
 * @brief Implementation of the UQ regular grid solver initialization routine.
 * @details Reads NetCDF-UGRID grid geometry and metadata using readUQgrid,
 *          captures grid metadata, and prepares domain decomposition partition
 * structures.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-23
 * @date Last update : 2026-11-03
 */

#include "ww4_core/solver_UQ/solver_UQ_init.h"
#include "ww4_core/solver_UQ/UQ_domains.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_read_grid.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_read_partition.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_write_partition.h"
#include "ww4_core/w4core_init.h"
#include "ww4_utils/spatial_grid.h"
#include "ww4_utils/ww4_logfile.h"
#include "ww4_utils/ww4_run_config.h"
#include "ww4_utils/ww4_std_out.h"
#include <filesystem>
#include <iostream>

namespace ww4_core {

// --- solver_UQ_init ---------------------------------------------------------

void solver_UQ_init(std::ostream &os) {
  if (getRunConfig().produceStdOut) {
    os << "  Processing the grid:\n"
       << "     Grid file                : " << getRunConfig().gridFile
       << std::endl;
  }
  if (getRunConfig().produceLogFile && getLogFileStream().is_open()) {
    getLogFileStream() << "  Processing the grid:\n"
                       << "     Grid file                : "
                       << getRunConfig().gridFile << std::endl;
  }
  // === Spatial grid ingestion ===============================================

  // Create UGRID data structure for reading grid geometry and metadata
  ww4::io::UgridGrid ugrid;

  // Attempt to read the file (all data)
  int status = ww4::io::readUQgrid(getRunConfig().gridFile, ugrid);

  // Error processing
  if (status != 0) {
    std::string errMsg;
    if (status == 1) {
      errMsg = "Grid file does not exist or failed opening: " +
               getRunConfig().gridFile;
    } else if (status == 2) {
      errMsg = "Failed reading metadata dimensions from file: " +
               getRunConfig().gridFile;
    } else {
      errMsg = "Failed reading grid dataset arrays from file: " +
               getRunConfig().gridFile;
    }
    // Explanatory comment preceding first use of __FILE__ and __LINE__
    // Pass source location for error tracing
    if (getRunConfig().produceLogFile && getLogFileStream().is_open()) {
      ww4_utils::ww4_std_out::writeExtcdeOutput(getLogFileStream(), errMsg,
                                                __FILE__, __LINE__);
    }
    // Failure to read file, abort program
    ww4_utils::ww4_std_out::extcde(status, os, errMsg, __FILE__, __LINE__);
  }

  // Output of meta data if requested
  if (getRunConfig().printGridMeta) {
    if (getRunConfig().produceStdOut) {
      ww4::io::printGridMetadata(ugrid, os);
    }
    if (getRunConfig().produceLogFile && getLogFileStream().is_open()) {
      ww4::io::printGridMetadata(ugrid, getLogFileStream());
    }
  }

  // Initialize FullGrid structure and general grid metadata
  ww4_utils::FullGrid fullGrid;
  fullGrid.clear();
  fullGrid.gridType = ww4_utils::GridType::Rectangular;
  fullGrid.shared.gridType = ww4_utils::GridType::Rectangular;
  fullGrid.name = ugrid.title.empty() ? getRunConfig().gridFile : ugrid.title;
  fullGrid.shared.name = fullGrid.name;
  fullGrid.nx = ugrid.nx;
  fullGrid.ny = ugrid.ny;
  fullGrid.numNodes = ugrid.nx * ugrid.ny;
  fullGrid.shared.numNodes = fullGrid.numNodes;

  // Populate 1D grid longitude and latitude coordinate arrays
  if (ugrid.nx > 0 && ugrid.ny > 0) {
    fullGrid.xCoordinates.reserve(ugrid.nx);
    for (std::size_t i = 0; i < ugrid.nx; ++i) {
      fullGrid.xCoordinates.push_back(ugrid.grid_lon[i]);
    }
    fullGrid.shared.xCoordinates = fullGrid.xCoordinates;

    fullGrid.yCoordinates.reserve(ugrid.ny);
    for (std::size_t j = 0; j < ugrid.ny; ++j) {
      fullGrid.yCoordinates.push_back(ugrid.grid_lat[j * ugrid.nx]);
    }
    fullGrid.shared.yCoordinates = fullGrid.yCoordinates;
  }

  // Populate rectangular grid specific data and check longitudinal closure
  ww4_utils::RectangularGridData rectData;
  rectData.nx = ugrid.nx;
  rectData.ny = ugrid.ny;
  if (!fullGrid.xCoordinates.empty()) {
    rectData.x0 = fullGrid.xCoordinates.front();
    if (ugrid.nx > 1) {
      rectData.dx =
          (fullGrid.xCoordinates.back() - fullGrid.xCoordinates.front()) /
          static_cast<double>(ugrid.nx - 1);
    }
  }
  if (!fullGrid.yCoordinates.empty()) {
    rectData.y0 = fullGrid.yCoordinates.front();
    if (ugrid.ny > 1) {
      rectData.dy =
          (fullGrid.yCoordinates.back() - fullGrid.yCoordinates.front()) /
          static_cast<double>(ugrid.ny - 1);
    }
  }
  rectData.isGlobal =
      (ugrid.global_closure == "closed" || ugrid.global_closure == "true" ||
       ugrid.global_closure == "yes" || ugrid.global_closure == "1");
  fullGrid.specific = rectData;

  // Populate unified topobathy elevation array
  fullGrid.topobathy.resize(fullGrid.numNodes);
  for (std::size_t i = 0; i < fullGrid.numNodes; ++i) {
    fullGrid.topobathy[i] = static_cast<double>(ugrid.grid_depth[i]);
  }
  fullGrid.shared.topobathy = fullGrid.topobathy;

  // Populate land-sea mask array
  fullGrid.landMask.resize(fullGrid.numNodes);
  for (std::size_t i = 0; i < fullGrid.numNodes; ++i) {
    fullGrid.landMask[i] = static_cast<int>(ugrid.grid_mask[i]);
  }
  fullGrid.shared.landMask = fullGrid.landMask;

  // Populate optional subgrid obstruction arrays in X and Y directions
  if (ugrid.has_obsx && !ugrid.grid_ObsX.empty()) {
    fullGrid.shared.obstructionX.resize(fullGrid.numNodes);
    for (std::size_t i = 0; i < fullGrid.numNodes; ++i) {
      fullGrid.shared.obstructionX[i] = static_cast<double>(ugrid.grid_ObsX[i]);
    }
  }
  if (ugrid.has_obsy && !ugrid.grid_ObsY.empty()) {
    fullGrid.shared.obstructionY.resize(fullGrid.numNodes);
    for (std::size_t i = 0; i < fullGrid.numNodes; ++i) {
      fullGrid.shared.obstructionY[i] = static_cast<double>(ugrid.grid_ObsY[i]);
    }
  }

  // Suppress unused variable warning pending domain decomposition integration
  (void)fullGrid;

  // === Domain decomposition setup ==========================================

  // Determine partition file name by adding ".partitions" before file extension
  std::filesystem::path gridFilePath(getRunConfig().gridFile);
  std::string partFileName = gridFilePath.stem().string() + ".partitions" +
                             gridFilePath.extension().string();
  std::filesystem::path partFilePath =
      gridFilePath.has_parent_path()
          ? (gridFilePath.parent_path() / partFileName)
          : std::filesystem::path(partFileName);
  std::string partFileStr = partFilePath.string();

  // Print partition file name to stdout and log file
  if (getRunConfig().produceStdOut) {
    os << "     Partition file           : " << partFileStr << std::endl;
  }
  if (getRunConfig().produceLogFile && getLogFileStream().is_open()) {
    getLogFileStream() << "     Partition file           : " << partFileStr
                       << std::endl;
  }

  // Attempt to read partition file
  ww4::io::PartitionMapData partData;
  int partStatus = ww4::io::readUQpartitionMapNetCDF(partFilePath, partData);

  // Check if partition count matches the user request
  if (partStatus == 0 &&
      partData.num_partitions != getRunConfig().numRanksCompute) {
    partStatus = 4;
  }

  std::vector<int> partitionMap;

  // Process read result: on failure, report error message and compute partition
  // map
  if (partStatus != 0) {
    std::string errMsg;
    if (partStatus == 1) {
      errMsg = "Partition file does not exist or failed opening";
    } else if (partStatus == 2) {
      errMsg = "Failed reading metadata dimensions from file";
    } else if (partStatus == 3) {
      errMsg = "Failed reading partition map dataset arrays from file";
    } else {
      errMsg = "Number of partitions in file does not match user request";
    }

    if (getRunConfig().produceStdOut) {
      os << "     Partition read error     : " << errMsg << std::endl;
    }
    if (getRunConfig().produceLogFile && getLogFileStream().is_open()) {
      getLogFileStream() << "     Partition read error     : " << errMsg
                         << std::endl;
    }

    // Report partition map creation to stdout and log file
    const std::string createMsg =
        "Creating the partition map for " +
        std::to_string(getRunConfig().numRanksCompute) + " partitions";
    if (getRunConfig().produceStdOut) {
      os << "     " << createMsg << std::endl;
    }
    if (getLogFileStream().is_open() && getRunConfig().produceLogFile) {
      getLogFileStream() << "     " << createMsg << std::endl;
    }

    // Compute partition map using domain partitioning routine
    partitionMap = ww4_utils::UQ_utils::partitionGlobalGrid(
        static_cast<int>(ugrid.nx), static_cast<int>(ugrid.ny),
        fullGrid.landMask, getRunConfig().numRanksCompute, 100, nullptr,
        rectData.isGlobal);

    // If partition map is successfully created, write a new partition file
    if (!partitionMap.empty()) {
      std::vector<int32_t> map32(partitionMap.begin(), partitionMap.end());
      int writeStatus = ww4::io::writeUQpartitionMapNetCDF(
          partFilePath, ugrid, getRunConfig().numRanksCompute, map32);
      if (writeStatus == 0) {
        const std::string writeMsg =
            "Writing partition file with 1 partition map";
        if (getRunConfig().produceStdOut) {
          os << "     " << writeMsg << std::endl;
        }
        if (getLogFileStream().is_open() && getRunConfig().produceLogFile) {
          getLogFileStream() << "     " << writeMsg << std::endl;
        }
      }
    }
  } else {
    // Populate partition map from successfully read file
    partitionMap.assign(partData.partition_map.begin(),
                        partData.partition_map.end());
  }

  // Suppress unused variable warning pending sub-domain extraction integration
  (void)partitionMap;

  // === Halo communication setup =============================================
  // Placeholder for initializing halo communication structures:
  // - Identify neighbor MPI ranks for sub-domain boundaries in X and Y
  // directions.
  // - Allocate ghost/halo cell send and receive buffers for boundary grid rows
  //   and columns.
  // - Pre-register non-blocking MPI communication handles and data types for
  //   efficient halo exchange during wave action propagation steps.
}

} // namespace ww4_core
