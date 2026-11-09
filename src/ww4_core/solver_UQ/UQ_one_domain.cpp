/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file UQ_one_domain.cpp
 * @brief Subdomain extraction routine for ultimate quickest (UQ) grids in
 * WAVEWATCH IV (WW4).
 * @details Extracts a reduced grid domain partition including halo cells and
 * associated grid fields for a target partition (derived from WW3 ww3_gspl).
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

#include "ww4_core/solver_UQ/UQ_one_domain.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <span>
#include <vector>

namespace ww4_core::solver_UQ {

// --- extractOneDomain -------------------------------------------------------

ww4_utils::LocalGridPartition
extractOneDomain(const ww4_utils::FullGrid &full_grid,
                 std::span<const int> partition_map, int target_partition,
                 int halo_width, bool is_global,
                 std::vector<int> *sliced_partition_map) {
  // === Section: Input Validation & Initial State Setup ======================
  ww4_utils::LocalGridPartition localPartition;
  localPartition.clear();
  localPartition.gridType = ww4_utils::GridType::Rectangular;
  localPartition.shared.gridType = ww4_utils::GridType::Rectangular;
  localPartition.rankId = (target_partition > 0) ? (target_partition - 1) : 0;
  const int halo = std::max(0, halo_width);
  localPartition.haloSizeX = halo;
  localPartition.haloSizeY = halo;

  const int nx = static_cast<int>(full_grid.nx);
  const int ny = static_cast<int>(full_grid.ny);

  bool grid_is_global = is_global;
  if (const auto *rectData = full_grid.getRectangularData()) {
    if (rectData->isGlobal) {
      grid_is_global = true;
    }
  }

  // Validate global grid dimensions, target partition ID, and partition map
  // size
  if (nx <= 0 || ny <= 0 || target_partition <= 0 ||
      partition_map.size() != static_cast<size_t>(nx * ny)) {
    return localPartition;
  }

  // === Section: Bounding Box & Occupied Columns Search ====================
  // Scan global 2D grid to record latitudinal row bounds (y_min, y_max) and
  // gather all longitudinal x-column indices containing target_partition cells
  std::vector<int> present_x_vec;
  int y_min = ny;
  int y_max = -1;

  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      int idx = y * nx + x;
      if (partition_map[idx] == target_partition) {
        present_x_vec.push_back(x);
        y_min = std::min(y_min, y);
        y_max = std::max(y_max, y);
      }
    }
  }

  // If target_partition is not found anywhere in the partition map, return
  // empty reduced grid
  if (present_x_vec.empty() || y_max < y_min) {
    return localPartition;
  }

  // Sort and remove duplicate x-column indices
  std::sort(present_x_vec.begin(), present_x_vec.end());
  present_x_vec.erase(std::unique(present_x_vec.begin(), present_x_vec.end()),
                      present_x_vec.end());

  int x_start = 0;
  int nx_loc = 0;

  // === Section: Longitudinal Bounds & Halo Expansion ========================
  if (grid_is_global && nx > 1) {
    // For global grids (360-degree longitudinal closure): analyze gap between
    // consecutive occupied x-columns to detect wraparound across x = 0 and x =
    // nx - 1.
    size_t m = present_x_vec.size();
    int max_gap = -1;
    size_t max_gap_idx = 0;

    for (size_t i = 0; i < m - 1; ++i) {
      int gap = present_x_vec[i + 1] - present_x_vec[i] - 1;
      if (gap > max_gap) {
        max_gap = gap;
        max_gap_idx = i;
      }
    }
    int wrap_gap = (present_x_vec[0] + nx) - present_x_vec[m - 1] - 1;
    if (wrap_gap > max_gap) {
      max_gap_idx = m - 1;
    }

    if (max_gap_idx == m - 1) {
      // Partition is contiguous in x without wrapping across boundary
      int x_min_occ = present_x_vec[0];
      int x_max_occ = present_x_vec[m - 1];
      int x_start_raw = x_min_occ - halo;
      int x_end_raw = x_max_occ + halo;
      nx_loc = x_end_raw - x_start_raw + 1;
      if (nx_loc >= nx) {
        nx_loc = nx;
        x_start = 0;
      } else {
        x_start = (x_start_raw % nx + nx) % nx;
      }
    } else {
      // Partition wraps across 360-degree longitudinal boundary (x = 0 / x = nx
      // - 1)
      int x_start_occ = present_x_vec[max_gap_idx + 1];
      int x_end_occ = present_x_vec[max_gap_idx];
      int occ_span = (x_end_occ + nx) - x_start_occ + 1;
      nx_loc = occ_span + 2 * halo;
      if (nx_loc >= nx) {
        nx_loc = nx;
        x_start = 0;
      } else {
        x_start = ((x_start_occ - halo) % nx + nx) % nx;
      }
    }
  } else {
    // Standard regional / non-global grid bounding box determination
    int x_min_occ = present_x_vec[0];
    int x_max_occ = present_x_vec.back();
    x_start = std::max(0, x_min_occ - halo);
    int x_end = std::min(nx - 1, x_max_occ + halo);
    nx_loc = x_end - x_start + 1;
  }

  // === Section: Latitudinal Bounds & Local Dimensions Setup =================
  int y_start = std::max(0, y_min - halo);
  int y_end = std::min(ny - 1, y_max + halo);

  localPartition.localNx = static_cast<std::size_t>(nx_loc);
  localPartition.localNy = static_cast<std::size_t>(y_end - y_start + 1);
  localPartition.localNumNodes =
      localPartition.localNx * localPartition.localNy;

  const int nx_loc_sz = nx_loc;
  const int ny_loc = y_end - y_start + 1;
  const size_t total_local_cells = static_cast<size_t>(nx_loc_sz * ny_loc);

  // === Section: 2D Field Slicing ============================================
  localPartition.landMask.resize(total_local_cells, 0);
  localPartition.localToGlobalMap.resize(total_local_cells, 0);

  if (sliced_partition_map != nullptr) {
    sliced_partition_map->resize(total_local_cells, 0);
  }

  const auto land_mask_span = full_grid.getLandMask();
  const bool has_ls = (land_mask_span.size() == static_cast<size_t>(nx * ny));

  for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
    int gy = y_start + y_loc;
    for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
      int gx = (x_start + x_loc) % nx;
      int g_idx = gy * nx + gx;
      int l_idx = y_loc * nx_loc_sz + x_loc;

      localPartition.localToGlobalMap[l_idx] = static_cast<std::size_t>(g_idx);
      if (sliced_partition_map != nullptr) {
        (*sliced_partition_map)[l_idx] = partition_map[g_idx];
      }
      if (has_ls) {
        localPartition.landMask[l_idx] = land_mask_span[g_idx];
      }
    }
  }
  localPartition.shared.landMask = localPartition.landMask;

  // Slice unified topobathy elevation field
  const auto topobathy_span = full_grid.getTopobathy();
  if (topobathy_span.size() == static_cast<size_t>(nx * ny)) {
    localPartition.topobathy.resize(total_local_cells, 0.0);
    for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
      int gy = y_start + y_loc;
      for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
        int gx = (x_start + x_loc) % nx;
        localPartition.topobathy[y_loc * nx_loc_sz + x_loc] =
            topobathy_span[gy * nx + gx];
      }
    }
    localPartition.shared.topobathy = localPartition.topobathy;
  }

  // Slice subgrid obstruction factors X and Y
  const auto obs_x_span = full_grid.shared.getObstructionX();
  if (obs_x_span.size() == static_cast<size_t>(nx * ny)) {
    localPartition.shared.obstructionX.resize(total_local_cells, 0.0);
    for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
      int gy = y_start + y_loc;
      for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
        int gx = (x_start + x_loc) % nx;
        localPartition.shared.obstructionX[y_loc * nx_loc_sz + x_loc] =
            obs_x_span[gy * nx + gx];
      }
    }
  }

  const auto obs_y_span = full_grid.shared.getObstructionY();
  if (obs_y_span.size() == static_cast<size_t>(nx * ny)) {
    localPartition.shared.obstructionY.resize(total_local_cells, 0.0);
    for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
      int gy = y_start + y_loc;
      for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
        int gx = (x_start + x_loc) % nx;
        localPartition.shared.obstructionY[y_loc * nx_loc_sz + x_loc] =
            obs_y_span[gy * nx + gx];
      }
    }
  }

  // === Section: Spatial Coordinate Slicing ==================================
  const auto lon_span = full_grid.getXCoordinates();
  if (lon_span.size() == static_cast<size_t>(nx)) {
    localPartition.xCoordinates.resize(nx_loc_sz);
    for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
      int gx = (x_start + x_loc) % nx;
      localPartition.xCoordinates[x_loc] = lon_span[gx];
    }
    localPartition.shared.xCoordinates = localPartition.xCoordinates;
  } else if (lon_span.size() == static_cast<size_t>(nx * ny)) {
    localPartition.xCoordinates.resize(total_local_cells);
    for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
      int gy = y_start + y_loc;
      for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
        int gx = (x_start + x_loc) % nx;
        localPartition.xCoordinates[y_loc * nx_loc_sz + x_loc] =
            lon_span[gy * nx + gx];
      }
    }
    localPartition.shared.xCoordinates = localPartition.xCoordinates;
  }

  const auto lat_span = full_grid.getYCoordinates();
  if (lat_span.size() == static_cast<size_t>(ny)) {
    localPartition.yCoordinates.resize(ny_loc);
    for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
      localPartition.yCoordinates[y_loc] = lat_span[y_start + y_loc];
    }
    localPartition.shared.yCoordinates = localPartition.yCoordinates;
  } else if (lat_span.size() == static_cast<size_t>(nx * ny)) {
    localPartition.yCoordinates.resize(total_local_cells);
    for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
      int gy = y_start + y_loc;
      for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
        int gx = (x_start + x_loc) % nx;
        localPartition.yCoordinates[y_loc * nx_loc_sz + x_loc] =
            lat_span[gy * nx + gx];
      }
    }
    localPartition.shared.yCoordinates = localPartition.yCoordinates;
  }

  return localPartition;
}

} // namespace ww4_core::solver_UQ
