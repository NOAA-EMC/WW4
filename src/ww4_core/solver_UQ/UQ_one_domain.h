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
 * @date Last update : 2026-11-09
 */

#pragma once

#include "ww4_utils/spatial_grid.h"
#include <span>
#include <vector>

namespace ww4_core::solver_UQ {

// --- extractOneDomain -------------------------------------------------------
/**
 * @brief Extracts a single reduced sub-domain partition grid with halo cells.
 * @details Pure subroutine that identifies the spatial bounding box of active
 * sea points assigned to target_partition in partition_map, expands the
 * bounding box by halo_width cells in all cardinal directions (clamped to
 * global grid boundaries or wrapped across 360-degree longitudinal boundary if
 * is_global is true), and slices all grid field datasets from full_grid into
 * local partition.
 *
 * Sub-Domain Extraction Algorithm Lifecycle:
 * Input Validation & Initial State Setup:
 *   Validates global grid dimensions (nx, ny > 0), target partition ID (> 0),
 *   and partition map array size. Checks whether global longitudinal closure
 *   flag is set either via full_grid or the is_global parameter.
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
 * @param[out] sliced_partition_map Optional vector to receive sliced local
 * partition IDs (size nx_local * ny_local).
 * @return LocalGridPartition structure containing extracted local sub-domain
 * grid and sliced data.
 */
ww4_utils::LocalGridPartition
extractOneDomain(const ww4_utils::FullGrid &full_grid,
                 std::span<const int> partition_map, int target_partition,
                 int halo_width = 1, bool is_global = false,
                 std::vector<int> *sliced_partition_map = nullptr);

} // namespace ww4_core::solver_UQ

namespace ww4_utils::UQ_utils {
// Backward-compatibility namespace alias
using ww4_core::solver_UQ::extractOneDomain;
} // namespace ww4_utils::UQ_utils
