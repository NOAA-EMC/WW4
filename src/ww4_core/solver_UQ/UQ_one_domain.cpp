/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file UQ_one_domain.cpp
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

#include "ww4_core/solver_UQ/UQ_one_domain.h"

#include <algorithm>
#include <span>
#include <vector>

namespace ww4_utils {
namespace UQ_utils {

// --- extractOneDomain -------------------------------------------------------

ReducedGrid extractOneDomain(const FullGrid &full_grid,
                             std::span<const int> partition_map,
                             int target_partition, int halo_width,
                             bool is_global) {
  // === Section: Input Validation & Initial State Setup ======================
  ReducedGrid reduced;
  reduced.target_partition = target_partition;
  reduced.halo_width = std::max(0, halo_width);

  const int nx = full_grid.nx;
  const int ny = full_grid.ny;
  const bool use_global = (is_global || full_grid.is_global);

  // Validate global grid dimensions, target partition ID, and partition map
  // size
  if (nx <= 0 || ny <= 0 || target_partition <= 0 ||
      partition_map.size() != static_cast<size_t>(nx * ny)) {
    return reduced;
  }

  // === Section: Bounding Box & Occupied Columns Search ====================
  // Scan global 2D grid to record latitudinal row bounds (y_min, y_max) and
  // gather all longitudinal x-column indices containing target_partition cells
  std::vector<int> present_x_vec;
  int y_min = ny, y_max = -1;

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
    return reduced;
  }

  // Sort and remove duplicate x-column indices
  std::sort(present_x_vec.begin(), present_x_vec.end());
  present_x_vec.erase(std::unique(present_x_vec.begin(), present_x_vec.end()),
                      present_x_vec.end());

  int x_start = 0;
  int x_end = 0;
  int nx_loc = 0;

  // === Section: Longitudinal Bounds & Halo Expansion ========================
  if (use_global && nx > 1) {
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
      int x_start_raw = x_min_occ - reduced.halo_width;
      int x_end_raw = x_max_occ + reduced.halo_width;
      nx_loc = x_end_raw - x_start_raw + 1;
      if (nx_loc >= nx) {
        nx_loc = nx;
        x_start = 0;
        x_end = nx - 1;
      } else {
        x_start = (x_start_raw % nx + nx) % nx;
        x_end = (x_end_raw % nx + nx) % nx;
      }
    } else {
      // Partition wraps across 360-degree longitudinal boundary (x = 0 / x = nx
      // - 1)
      int x_start_occ = present_x_vec[max_gap_idx + 1];
      int x_end_occ = present_x_vec[max_gap_idx];
      int occ_span = (x_end_occ + nx) - x_start_occ + 1;
      nx_loc = occ_span + 2 * reduced.halo_width;
      if (nx_loc >= nx) {
        nx_loc = nx;
        x_start = 0;
        x_end = nx - 1;
      } else {
        x_start = ((x_start_occ - reduced.halo_width) % nx + nx) % nx;
        x_end = ((x_end_occ + reduced.halo_width) % nx + nx) % nx;
      }
    }
  } else {
    // Standard regional / non-global grid bounding box determination
    int x_min_occ = present_x_vec[0];
    int x_max_occ = present_x_vec.back();
    x_start = std::max(0, x_min_occ - reduced.halo_width);
    x_end = std::min(nx - 1, x_max_occ + reduced.halo_width);
    nx_loc = x_end - x_start + 1;
  }

  // === Section: Latitudinal Bounds & Local Dimensions Setup =================
  int y_start = std::max(0, y_min - reduced.halo_width);
  int y_end = std::min(ny - 1, y_max + reduced.halo_width);

  reduced.global_x_min = x_start;
  reduced.global_y_min = y_start;
  reduced.global_x_max = x_end;
  reduced.global_y_max = y_end;

  reduced.nx_local = nx_loc;
  reduced.ny_local = y_end - y_start + 1;

  const int nx_loc_sz = reduced.nx_local;
  const int ny_loc = reduced.ny_local;
  const size_t total_local_cells = static_cast<size_t>(nx_loc_sz * ny_loc);

  // === Section: 2D Field Slicing ============================================
  // Slice 2D partition map and land-sea status mask into row-major local grid
  reduced.land_sea_map.resize(total_local_cells, 0);
  reduced.partition_map.resize(total_local_cells, 0);

  const bool has_ls =
      (full_grid.land_sea_map.size() == static_cast<size_t>(nx * ny));

  for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
    int gy = y_start + y_loc;
    for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
      int gx = (x_start + x_loc) % nx;
      int g_idx = gy * nx + gx;
      int l_idx = y_loc * nx_loc_sz + x_loc;

      reduced.partition_map[l_idx] = partition_map[g_idx];
      if (has_ls) {
        reduced.land_sea_map[l_idx] = full_grid.land_sea_map[g_idx];
      }
    }
  }

  // Slice unified topobathy elevation field
  if (full_grid.topobathy.size() == static_cast<size_t>(nx * ny)) {
    reduced.topobathy.resize(total_local_cells, 0.0);
    for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
      int gy = y_start + y_loc;
      for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
        int gx = (x_start + x_loc) % nx;
        reduced.topobathy[y_loc * nx_loc_sz + x_loc] =
            full_grid.topobathy[gy * nx + gx];
      }
    }
  }

  // Slice subgrid obstruction factors X and Y
  if (full_grid.subgrid_obs_x.size() == static_cast<size_t>(nx * ny)) {
    reduced.subgrid_obs_x.resize(total_local_cells, 0.0);
    for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
      int gy = y_start + y_loc;
      for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
        int gx = (x_start + x_loc) % nx;
        reduced.subgrid_obs_x[y_loc * nx_loc_sz + x_loc] =
            full_grid.subgrid_obs_x[gy * nx + gx];
      }
    }
  }

  if (full_grid.subgrid_obs_y.size() == static_cast<size_t>(nx * ny)) {
    reduced.subgrid_obs_y.resize(total_local_cells, 0.0);
    for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
      int gy = y_start + y_loc;
      for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
        int gx = (x_start + x_loc) % nx;
        reduced.subgrid_obs_y[y_loc * nx_loc_sz + x_loc] =
            full_grid.subgrid_obs_y[gy * nx + gx];
      }
    }
  }

  // === Section: Spatial Coordinate Slicing ==================================
  // Slice longitude coordinates (supporting 1D vector or 2D array
  // representation)
  if (full_grid.longitude.size() == static_cast<size_t>(nx)) {
    reduced.longitude.resize(nx_loc_sz);
    for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
      int gx = (x_start + x_loc) % nx;
      reduced.longitude[x_loc] = full_grid.longitude[gx];
    }
  } else if (full_grid.longitude.size() == static_cast<size_t>(nx * ny)) {
    reduced.longitude.resize(total_local_cells);
    for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
      int gy = y_start + y_loc;
      for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
        int gx = (x_start + x_loc) % nx;
        reduced.longitude[y_loc * nx_loc_sz + x_loc] =
            full_grid.longitude[gy * nx + gx];
      }
    }
  }

  // Slice latitude coordinates (supporting 1D vector or 2D array
  // representation)
  if (full_grid.latitude.size() == static_cast<size_t>(ny)) {
    reduced.latitude.resize(ny_loc);
    for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
      reduced.latitude[y_loc] = full_grid.latitude[y_start + y_loc];
    }
  } else if (full_grid.latitude.size() == static_cast<size_t>(nx * ny)) {
    reduced.latitude.resize(total_local_cells);
    for (int y_loc = 0; y_loc < ny_loc; ++y_loc) {
      int gy = y_start + y_loc;
      for (int x_loc = 0; x_loc < nx_loc_sz; ++x_loc) {
        int gx = (x_start + x_loc) % nx;
        reduced.latitude[y_loc * nx_loc_sz + x_loc] =
            full_grid.latitude[gy * nx + gx];
      }
    }
  }

  return reduced;
}

} // namespace UQ_utils
} // namespace ww4_utils
