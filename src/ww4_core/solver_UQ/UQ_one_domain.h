/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file UQ_one_domain.h
 * @brief Subdomain extraction routine for ultimate quickest (UQ) grids in
 * WAVEWATCH IV (WW4).
 * @details Provides a pure subroutine to extract a reduced grid domain
 * partition including halo cells and associated grid fields for a target
 * partition (derived from WW3 ww3_gspl).
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

#pragma once

#include <span>
#include <vector>

namespace ww4_utils {
namespace UQ_utils {

// --- FullGrid ---------------------------------------------------------------
/**
 * @struct FullGrid
 * @brief Structure representing a full global grid dataset and associated
 * spatial fields.
 * @details Serves as an input container for global spatial grid parameters,
 * land-sea status mask, 1D or 2D geographic coordinates, unified topobathy
 * elevation, and subgrid obstruction factors prior to sub-domain extraction.
 */
struct FullGrid {
  int nx{0}; ///< Total longitudinal grid column count (x-dimension).
  int ny{0}; ///< Total latitudinal grid row count (y-dimension).
  bool is_global{false}; ///< Flag indicating whether grid has longitudinal
                         ///< closure (360-degree wrapping across x = 0 and x =
                         ///< nx - 1).
  std::vector<int>
      land_sea_map; ///< 2D land-sea status mask (row-major order, size nx * ny,
                    ///< > 0 for active sea cells, <= 0 for land).
  std::vector<double>
      longitude; ///< 1D longitude coordinates (size nx) or 2D (size nx * ny).
  std::vector<double>
      latitude; ///< 1D latitude coordinates (size ny) or 2D (size nx * ny).
  std::vector<double> topobathy; ///< Unified topobathy elevation array in
                                 ///< meters (row-major order, size nx * ny).
  std::vector<double>
      subgrid_obs_x; ///< Optional subgrid obstruction factors in X direction
                     ///< (row-major order, size nx * ny).
  std::vector<double>
      subgrid_obs_y; ///< Optional subgrid obstruction factors in Y direction
                     ///< (row-major order, size nx * ny).
};

// --- ReducedGrid ------------------------------------------------------------
/**
 * @struct ReducedGrid
 * @brief Structure representing a reduced sub-domain grid partition dataset.
 * @details Holds extracted spatial grid dimensions, global bounding box
 * coordinates, halo width metadata, and sliced local grid fields (land-sea
 * mask, partition IDs, geographic coordinates, topobathy elevation, and
 * subgrid obstructions) tailored for a target MPI partition.
 */
struct ReducedGrid {
  int target_partition{0}; ///< Target 1-based partition ID extracted.
  int nx_local{0};         ///< Local grid column count including halo cells.
  int ny_local{0};         ///< Local grid row count including halo cells.
  int global_x_min{0};     ///< 0-based global x index of local origin (0,0).
  int global_y_min{0};     ///< 0-based global y index of local origin (0,0).
  int global_x_max{
      0}; ///< 0-based global x index of local (nx_local-1, ny_local-1).
  int global_y_max{
      0}; ///< 0-based global y index of local (nx_local-1, ny_local-1).
  int halo_width{0}; ///< Halo overlap width in grid cells.
  std::vector<int>
      land_sea_map; ///< Sliced local land-sea status mask
                    ///< (row-major order, size nx_local * ny_local).
  std::vector<int>
      partition_map; ///< Sliced local partition map IDs
                     ///< (row-major order, size nx_local * ny_local).
  std::vector<double>
      longitude; ///< Sliced local longitude coordinates
                 ///< (size nx_local for 1D, nx_local * ny_local for 2D).
  std::vector<double>
      latitude; ///< Sliced local latitude coordinates
                ///< (size ny_local for 1D, nx_local * ny_local for 2D).
  std::vector<double>
      topobathy; ///< Sliced local unified topobathy elevation
                 ///< (row-major order, size nx_local * ny_local).
  std::vector<double>
      subgrid_obs_x; ///< Sliced local subgrid obstruction X
                     ///< (row-major order, size nx_local * ny_local).
  std::vector<double>
      subgrid_obs_y; ///< Sliced local subgrid obstruction Y
                     ///< (row-major order, size nx_local * ny_local).
};

// --- extractOneDomain -------------------------------------------------------
/**
 * @brief Extracts a single reduced sub-domain partition grid with halo cells.
 * @details Pure subroutine that identifies the spatial bounding box of active
 * sea points assigned to target_partition in partition_map, expands the
 * bounding box by halo_width cells in all cardinal directions (clamped to
 * global grid boundaries or wrapped across 360-degree longitudinal boundary if
 * is_global is true), and slices all grid field datasets from full_grid into
 * reduced_grid.
 *
 * Sub-Domain Extraction Algorithm Lifecycle:
 * Input Validation & Initial State Setup:
 *   Validates global grid dimensions (nx, ny > 0), target partition ID (> 0),
 *   and partition map array size. Checks whether global longitudinal closure
 *   flag is set either via full_grid.is_global or the is_global parameter.
 *
 * Bounding Box Search & Occupied Column Scan:
 *   Scans global 2D partition map for cells matching target_partition.
 *   Determines y_min and y_max latitudinal row bounds. Collects all occupied
 *   x-column indices into a sorted vector with duplicate values removed.
 *
 * Longitudinal Boundary Handling & Halo Expansion:
 *   For standard non-global grids:
 *     Calculates occupied longitude bounds x_min = min(present_x) and
 *     x_max = max(present_x). Expands bounding box by halo_width:
 *       x_start = max(0, x_min - halo_width)
 *       x_end   = min(nx - 1, x_max + halo_width)
 *       nx_local = x_end - x_start + 1
 *
 *   For global grids (360-degree longitudinal closure):
 *     Analyzes gaps between consecutive occupied x-columns to detect wrapping
 *     across x = 0 and x = nx - 1. Identifies the maximum unassigned gap in
 *     longitude index space. Sets local column origin x_start and local column
 *     count nx_local considering halo expansion with modular wraparound.
 *
 * Latitudinal Bounds Determination:
 *   Expands row bounds by halo_width with hard boundary clamping:
 *     y_start = max(0, y_min - halo_width)
 *     y_end   = min(ny - 1, y_max + halo_width)
 *     ny_local = y_end - y_start + 1
 *
 * Field Dataset Slicing:
 *   Allocates local memory buffers of size nx_local * ny_local.
 *   Slices 2D row-major mask arrays (land_sea_map, partition_map), topobathy
 *   elevation, and subgrid obstruction factors (subgrid_obs_x, subgrid_obs_y).
 *   Slices 1D vector or 2D array spatial coordinate fields (longitude,
 *   latitude), mapping global indices to local sub-domain indices.
 *
 * @param full_grid Full global grid structure containing spatial dimensions and
 * grid fields.
 * @param partition_map Global 2D partition map array (size full_grid.nx *
 * full_grid.ny).
 * @param target_partition Target 1-based partition ID to extract.
 * @param halo_width Halo overlap width in grid cells (default: 1).
 * @param is_global Flag indicating whether grid has longitudinal closure
 * (360-degree wrapping across x = 0 and x = nx - 1) (default: false).
 * @return ReducedGrid structure containing extracted local sub-domain grid and
 * sliced data.
 */
ReducedGrid extractOneDomain(const FullGrid &full_grid,
                             std::span<const int> partition_map,
                             int target_partition, int halo_width = 1,
                             bool is_global = false);

} // namespace UQ_utils
} // namespace ww4_utils
