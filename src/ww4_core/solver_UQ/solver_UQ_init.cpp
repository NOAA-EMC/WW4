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
 * @date Last update : 2026-10-30
 */

#include "ww4_core/solver_UQ/solver_UQ_init.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_read_grid.h"
#include "ww4_core/w4core_init.h"
#include "ww4_utils/spatial_grid.h"
#include "ww4_utils/ww4_logfile.h"
#include "ww4_utils/ww4_run_config.h"
#include "ww4_utils/ww4_std_out.h"
#include <iostream>

namespace ww4_core {

// --- solver_UQ_init ---------------------------------------------------------
void solver_UQ_init(std::ostream &os) {
  if (getRunConfig().produceStdOut) {
    os << "  Processing the grid:\n"
       << "     Grid file: " << getRunConfig().gridFile << std::endl;
  }
  if (getRunConfig().produceLogFile && getLogFileStream().is_open()) {
    getLogFileStream() << "  Processing the grid:\n"
                       << "     Grid file: " << getRunConfig().gridFile
                       << std::endl;
  }

  // === Spatial grid ingestion ===============================================
  ww4::io::UgridGrid ugrid;
  int status = ww4::io::readUQgrid(getRunConfig().gridFile, ugrid);
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
    ww4_utils::ww4_std_out::extcde(status, os, errMsg, __FILE__, __LINE__);
  }

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

  fullGrid.topobathy.resize(fullGrid.numNodes);
  for (std::size_t i = 0; i < fullGrid.numNodes; ++i) {
    fullGrid.topobathy[i] = static_cast<double>(ugrid.grid_depth[i]);
  }
  fullGrid.shared.topobathy = fullGrid.topobathy;

  fullGrid.landMask.resize(fullGrid.numNodes);
  for (std::size_t i = 0; i < fullGrid.numNodes; ++i) {
    fullGrid.landMask[i] = static_cast<int>(ugrid.grid_mask[i]);
  }
  fullGrid.shared.landMask = fullGrid.landMask;

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

  (void)fullGrid;

  // === Domain decomposition and partition data lifecycle ===================
  // Placeholder for grid partitioning and parallel sub-domain setup:
  // - Check runtime partition mode configuration (grid.partition.mode).
  // - If precomputed partition file exists (e.g., partition_map.nc):
  //     * Call PartitionIO::readPartitionMapNetCDF to load domain assignments,
  //       local sub-grid bounding boxes (nx_local, ny_local), and halo exchange
  //       maps.
  // - Else (generate_internal or auto mode):
  //     * Rank 0 / Initializer calls partitionGlobalGrid (2D rectilinear band
  //     or
  //       METIS/Cardigan domain partitioner) to assign grid cells to MPI ranks.
  //     * Export partition map to file via PartitionIO::writePartitionMapNetCDF
  //       (partition_map.nc) for future execution runs.
  // - Populate LocalGridPartition for current rank, initializing local
  // contiguous
  //   topobathy buffers, ghost/halo boundary cell mappings, and
  //   local_to_global_map.

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
