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
 * @date Last update : 2026-11-06
 */

#include "ww4_core/solver_UQ/solver_UQ_init.h"
#include "ww4_core/solver_UQ/UQ_domains.h"
#include "ww4_core/solver_UQ/UQ_one_domain.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_read_grid.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_read_partition.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_write_grid.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_write_partition.h"
#include "ww4_core/w4core_init.h"
#include "ww4_utils/spatial_grid.h"
#include "ww4_utils/ww4_logfile.h"
#include "ww4_utils/ww4_run_config.h"
#include "ww4_utils/ww4_std_out.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

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

  // Validate that partition file metadata and dimensions match grid file
  if (partStatus == 0) {
    const std::size_t exp_faces =
        (ugrid.n_faces > 0) ? ugrid.n_faces : (ugrid.nx * ugrid.ny);
    const std::size_t exp_nodes =
        (ugrid.n_nodes > 0) ? ugrid.n_nodes : (ugrid.nx + 1) * (ugrid.ny + 1);
    const std::size_t exp_max_nodes =
        (ugrid.max_face_nodes > 0) ? ugrid.max_face_nodes : 4;

    const bool dimsMatch =
        (partData.nx == ugrid.nx && partData.ny == ugrid.ny &&
         partData.n_faces == exp_faces && partData.n_nodes == exp_nodes &&
         partData.max_face_nodes == exp_max_nodes);

    const bool metaMatch =
        (partData.title == ugrid.title && partData.source == ugrid.source &&
         partData.grid_type == ugrid.grid_type &&
         partData.coord_type == ugrid.coord_type &&
         partData.global_closure == ugrid.global_closure &&
         partData.bathy_source == ugrid.bathy_source &&
         partData.bound_source == ugrid.bound_source &&
         partData.conventions == ugrid.conventions);

    if (!dimsMatch || !metaMatch) {
      partStatus = 5;
    } else if (partData.num_partitions != getRunConfig().numRanksCompute) {
      partStatus = 4;
    }
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
    } else if (partStatus == 5) {
      errMsg = "Metadata in partition file does not match grid file";
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

    int ncid = -1;
    std::size_t map_index = 0;
    std::vector<int32_t> lastWrittenMap;
    std::function<void(std::span<const int>)> recorder = nullptr;

    if (getRunConfig().intermediateMaps) {
      static_cast<void>(ww4::io::openUQpartitionMapFile(
          partFilePath, ugrid, getRunConfig().numRanksCompute, ncid));
      if (ncid >= 0) {
        recorder = [&](std::span<const int> curMap) {
          std::vector<int32_t> map32(curMap.begin(), curMap.end());
          if (lastWrittenMap.empty() || lastWrittenMap != map32) {
            int write_status =
                ww4::io::writeSingleUQpartitionMap(ncid, map32, map_index);
            if (write_status == 0) {
              lastWrittenMap = std::move(map32);
            }
          }
        };
      }
    }

    // Compute partition map using domain partitioning routine
    partitionMap = ww4_utils::UQ_utils::partitionGlobalGrid(
        static_cast<int>(ugrid.nx), static_cast<int>(ugrid.ny),
        fullGrid.landMask, getRunConfig().numRanksCompute,
        getRunConfig().maxIterations, nullptr, rectData.isGlobal, recorder);

    // If partition map is successfully created, write or finalize partition
    // file
    if (!partitionMap.empty()) {
      std::vector<int32_t> map32(partitionMap.begin(), partitionMap.end());

      if (getRunConfig().intermediateMaps) {
        if (ncid >= 0) {
          // Always write the final mask if not already the last written mask
          if (lastWrittenMap.empty() || lastWrittenMap != map32) {
            static_cast<void>(
                ww4::io::writeSingleUQpartitionMap(ncid, map32, map_index));
            lastWrittenMap = map32;
          }
          static_cast<void>(ww4::io::closeUQpartitionMapFile(ncid));

          const std::string writeMsg =
              "Writing partition file with " + std::to_string(map_index) +
              " partition " + (map_index == 1 ? "map" : "maps");
          if (getRunConfig().produceStdOut) {
            os << "     " << writeMsg << std::endl;
          }
          if (getLogFileStream().is_open() && getRunConfig().produceLogFile) {
            getLogFileStream() << "     " << writeMsg << std::endl;
          }
        }
      } else {
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

      // Compute final domain sizes (active wet points per partition) and output
      // histogram only when a new partition map is generated
      if (getRunConfig().numRanksCompute > 0) {
        int totalActiveWetPoints = 0;
        std::vector<int> domainSizes(getRunConfig().numRanksCompute + 1, 0);
        for (size_t i = 0; i < partitionMap.size(); ++i) {
          int pid = partitionMap[i];
          if (pid >= 1 && pid <= getRunConfig().numRanksCompute &&
              (fullGrid.landMask[i] == 1 || fullGrid.landMask[i] == 2)) {
            domainSizes[pid]++;
            totalActiveWetPoints++;
          }
        }

        // Sort domain sizes and group per size in histogram format
        std::map<int, int> sizeHistogram;
        for (int p = 1; p <= getRunConfig().numRanksCompute; ++p) {
          sizeHistogram[domainSizes[p]]++;
        }

        double meanSize = static_cast<double>(totalActiveWetPoints) /
                          static_cast<double>(getRunConfig().numRanksCompute);

        auto printHistogram = [&](std::ostream &out) {
          out << "     Final domain sizes (mean: " << std::fixed
              << std::setprecision(1) << meanSize << "):" << std::endl;
          for (const auto &[sz, count] : sizeHistogram) {
            out << "        " << std::setw(6) << sz
                << " points : " << std::string(count, '*') << " (" << count
                << ")" << std::endl;
          }
        };

        if (getRunConfig().produceStdOut) {
          printHistogram(os);
        }
        if (getLogFileStream().is_open() && getRunConfig().produceLogFile) {
          printHistogram(getLogFileStream());
        }
      }
    } else if (ncid >= 0) {
      static_cast<void>(ww4::io::closeUQpartitionMapFile(ncid));
    }
  } else {
    // Populate partition map from successfully read file
    partitionMap.assign(partData.partition_map.begin(),
                        partData.partition_map.end());
  }

  // === Extract local grid ===================================================

  // Prepare FullGrid structure for single subdomain extraction
  ww4_utils::UQ_utils::FullGrid uqFullGrid;
  uqFullGrid.nx = static_cast<int>(ugrid.nx);
  uqFullGrid.ny = static_cast<int>(ugrid.ny);
  uqFullGrid.is_global = rectData.isGlobal;
  uqFullGrid.land_sea_map = fullGrid.landMask;
  uqFullGrid.longitude = fullGrid.xCoordinates;
  uqFullGrid.latitude = fullGrid.yCoordinates;
  uqFullGrid.topobathy = fullGrid.topobathy;
  if (!fullGrid.shared.obstructionX.empty()) {
    uqFullGrid.subgrid_obs_x = fullGrid.shared.obstructionX;
  }
  if (!fullGrid.shared.obstructionY.empty()) {
    uqFullGrid.subgrid_obs_y = fullGrid.shared.obstructionY;
  }

  // Domain ID for actual compute rank (1-based ID matching partitionMap)
  const int targetDomainId = getRunConfig().actualRank + 1;
  constexpr int defaultHaloWidth = 2;

  // Extract reduced domain grid including halo cells
  ww4_utils::UQ_utils::ReducedGrid reducedGrid =
      ww4_utils::UQ_utils::extractOneDomain(uqFullGrid, partitionMap,
                                            targetDomainId, defaultHaloWidth,
                                            rectData.isGlobal);

  // Initialize and populate LocalGridPartition
  ww4_utils::LocalGridPartition localPartition;
  localPartition.clear();
  localPartition.gridType = ww4_utils::GridType::Rectangular;
  localPartition.shared.gridType = ww4_utils::GridType::Rectangular;
  localPartition.rankId = getRunConfig().actualRank;
  localPartition.localNx = reducedGrid.nx_local;
  localPartition.localNy = reducedGrid.ny_local;
  localPartition.localNumNodes =
      static_cast<std::size_t>(reducedGrid.nx_local * reducedGrid.ny_local);
  localPartition.haloSizeX = defaultHaloWidth;
  localPartition.haloSizeY = defaultHaloWidth;

  if (reducedGrid.nx_local > 0 && reducedGrid.ny_local > 0) {
    localPartition.xCoordinates = reducedGrid.longitude;
    localPartition.shared.xCoordinates = localPartition.xCoordinates;
    localPartition.yCoordinates = reducedGrid.latitude;
    localPartition.shared.yCoordinates = localPartition.yCoordinates;
    localPartition.topobathy = reducedGrid.topobathy;
    localPartition.shared.topobathy = localPartition.topobathy;

    // Build local domain grid mask:
    // 0 = Land (dry)
    // 1 = Sea / wet (active calculation points owned by target domain)
    // 2 = Boundary points
    // 3 = Excluded points (wet points outside domain and halo, or boundary
    //     points not adjacent to wet/active calculation points)
    const int loc_nx = reducedGrid.nx_local;
    const int loc_ny = reducedGrid.ny_local;
    const std::size_t loc_total = static_cast<std::size_t>(loc_nx * loc_ny);

    // Identify which cells in reducedGrid are owned by the target domain
    std::vector<bool> isTargetDomain(loc_total, false);
    for (std::size_t i = 0; i < loc_total; ++i) {
      if (reducedGrid.partition_map[i] == targetDomainId) {
        isTargetDomain[i] = true;
      }
    }

    // Identify halo points of halo width surrounding the target domain
    std::vector<bool> isHalo(loc_total, false);
    for (int y = 0; y < loc_ny; ++y) {
      for (int x = 0; x < loc_nx; ++x) {
        const int idx = y * loc_nx + x;
        if (isTargetDomain[idx]) {
          continue;
        }
        // Check Chebyshev distance to any cell in the target domain
        bool nearTarget = false;
        const int y_min_box = std::max(0, y - defaultHaloWidth);
        const int y_max_box = std::min(loc_ny - 1, y + defaultHaloWidth);
        const int x_min_box = std::max(0, x - defaultHaloWidth);
        const int x_max_box = std::min(loc_nx - 1, x + defaultHaloWidth);

        for (int by = y_min_box; by <= y_max_box && !nearTarget; ++by) {
          for (int bx = x_min_box; bx <= x_max_box; ++bx) {
            if (isTargetDomain[by * loc_nx + bx]) {
              nearTarget = true;
              break;
            }
          }
        }
        if (nearTarget) {
          isHalo[idx] = true;
        }
      }
    }

    std::vector<int> localMask(loc_total, 0);
    // Initial pass: populate base values and mark non-domain, non-halo wet
    // points as excluded (3)
    for (std::size_t i = 0; i < loc_total; ++i) {
      const int g_mask = reducedGrid.land_sea_map[i];
      if (g_mask == 0) {
        // Land remains land
        localMask[i] = 0;
      } else if (isTargetDomain[i]) {
        // Points belonging to target domain retain original mask (1 or 2)
        localMask[i] = g_mask;
      } else if (isHalo[i]) {
        // Halo wet or boundary points retain original status
        localMask[i] = g_mask;
      } else {
        // Wet points outside target domain and outside halo become excluded (3)
        localMask[i] = 3;
      }
    }

    // Second pass: boundary points (2) that are not adjacent to any wet/active
    // calculation points (localMask == 1) in the new grid become excluded (3)
    const int dx8[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    const int dy8[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    for (int y = 0; y < loc_ny; ++y) {
      for (int x = 0; x < loc_nx; ++x) {
        const int idx = y * loc_nx + x;
        if (localMask[idx] == 2) {
          bool adjToWet = false;
          for (int k = 0; k < 8; ++k) {
            const int nx_k = x + dx8[k];
            const int ny_k = y + dy8[k];
            if (nx_k >= 0 && nx_k < loc_nx && ny_k >= 0 && ny_k < loc_ny) {
              if (localMask[ny_k * loc_nx + nx_k] == 1) {
                adjToWet = true;
                break;
              }
            }
          }
          if (!adjToWet) {
            localMask[idx] = 3;
          }
        }
      }
    }

    localPartition.landMask = localMask;
    localPartition.shared.landMask = localPartition.landMask;

    // Optional writing of extracted local domain grid to NetCDF file
    if (getRunConfig().writeLocalGrid) {
      // Derive local domain grid filename: <BASE_NAME>_ugrid_part<NNN>.nc
      std::string stem = gridFilePath.stem().string();
      std::string baseName;
      if (stem.size() >= 6 && stem.rfind("_ugrid") == stem.size() - 6) {
        baseName = stem.substr(0, stem.size() - 6);
      } else {
        baseName = stem;
      }

      // Format zero-padded 0-based partition number <NNN>
      const int numRanks = getRunConfig().numRanksCompute;
      const int maxRank = std::max(0, numRanks - 1);
      int numDigits = 1;
      int tempVal = maxRank;
      while (tempVal >= 10) {
        tempVal /= 10;
        numDigits++;
      }
      const int padWidth = std::max(3, numDigits);

      std::ostringstream partNumOss;
      partNumOss << std::setw(padWidth) << std::setfill('0')
                 << getRunConfig().actualRank;
      const std::string localGridFileName =
          baseName + "_ugrid_part" + partNumOss.str() + ".nc";

      std::filesystem::path localGridPath =
          gridFilePath.has_parent_path()
              ? (gridFilePath.parent_path() / localGridFileName)
              : std::filesystem::path(localGridFileName);

      // Construct local UgridGrid for NetCDF export
      ww4::io::UgridGrid localUgrid;
      localUgrid.nx = static_cast<std::size_t>(loc_nx);
      localUgrid.ny = static_cast<std::size_t>(loc_ny);
      localUgrid.title = ugrid.title.empty() ? localGridFileName : ugrid.title;
      localUgrid.source = ugrid.source;
      localUgrid.grid_type = ugrid.grid_type;
      localUgrid.coord_type = ugrid.coord_type;
      localUgrid.global_closure = "none";
      localUgrid.bathy_source = ugrid.bathy_source;
      localUgrid.bound_source = ugrid.bound_source;
      localUgrid.conventions = ugrid.conventions;
      localUgrid.history = ugrid.history;
      localUgrid.global_attrs = ugrid.global_attrs;

      // 2D grid coordinates and fields
      localUgrid.grid_lon.resize(loc_total);
      localUgrid.grid_lat.resize(loc_total);
      for (int y = 0; y < loc_ny; ++y) {
        for (int x = 0; x < loc_nx; ++x) {
          const int idx = y * loc_nx + x;
          localUgrid.grid_lon[idx] = reducedGrid.longitude[x];
          localUgrid.grid_lat[idx] = reducedGrid.latitude[y];
        }
      }

      localUgrid.grid_depth.resize(loc_total);
      for (std::size_t i = 0; i < loc_total; ++i) {
        localUgrid.grid_depth[i] = static_cast<float>(reducedGrid.topobathy[i]);
      }

      localUgrid.grid_mask.resize(loc_total);
      for (std::size_t i = 0; i < loc_total; ++i) {
        localUgrid.grid_mask[i] = static_cast<int32_t>(localMask[i]);
      }

      if (!reducedGrid.subgrid_obs_x.empty()) {
        localUgrid.has_obsx = true;
        localUgrid.grid_ObsX.resize(loc_total);
        for (std::size_t i = 0; i < loc_total; ++i) {
          localUgrid.grid_ObsX[i] =
              static_cast<float>(reducedGrid.subgrid_obs_x[i]);
        }
      }

      if (!reducedGrid.subgrid_obs_y.empty()) {
        localUgrid.has_obsy = true;
        localUgrid.grid_ObsY.resize(loc_total);
        for (std::size_t i = 0; i < loc_total; ++i) {
          localUgrid.grid_ObsY[i] =
              static_cast<float>(reducedGrid.subgrid_obs_y[i]);
        }
      }

      const int writeStatus = ww4::io::writeUQgrid(localGridPath, localUgrid);
      if (writeStatus == 0) {
        const std::string localPathStr = localGridPath.string();
        if (getRunConfig().produceStdOut) {
          os << "     Local domain grid file   : " << localPathStr << std::endl;
        }
        if (getRunConfig().produceLogFile && getLogFileStream().is_open()) {
          getLogFileStream()
              << "     Local domain grid file   : " << localPathStr
              << std::endl;
        }
      }
    }
  }

  // Suppress unused variable warning pending subsequent solver stages
  (void)localPartition;

  // === Cross-mapping local and full grids ====================================
  // Placeholder for establishing bidirectional index mappings between full grid
  // storage arrays and local domain storage arrays:
  // - Generate 2D-to-1D and 1D-to-2D storage maps for active wet points.
  // - Correlate local domain wet points to global domain storage indices.

  // === Set up halo communication ============================================
  // Placeholder for initializing halo communication structures:
  // - Identify neighbor MPI ranks for sub-domain boundaries in X and Y
  // directions.
  // - Allocate ghost/halo cell send and receive buffers for boundary grid rows
  //   and columns.
  // - Pre-register non-blocking MPI communication handles and data types for
  //   efficient halo exchange during wave action propagation steps.
}

} // namespace ww4_core
