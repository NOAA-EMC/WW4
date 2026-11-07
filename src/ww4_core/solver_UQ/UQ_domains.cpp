/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file UQ_domains.cpp
 * @brief Domain partitioning routines for ultimate quickest (UQ) grids in
 * WAVEWATCH IV (WW4).
 * @details Provides pure subroutines to partition full UQ structured grids into
 * sub-domains based on active sea points (derived from WW3 ww3_gspl).
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
 * @date Initial, 2026-10-14
 * @date Last update : 2026-11-06
 */

#include "ww4_core/solver_UQ/UQ_domains.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <set>
#include <span>
#include <vector>

namespace ww4_utils {
namespace UQ_utils {

namespace {

// --- isWetPoint -------------------------------------------------------------
/**
 * @brief Checks if a grid cell is an active wet point.
 * @details Excluded points (value 3) and land points (<= 0) are excluded.
 * @param val Status mask value at grid cell.
 * @return True if cell is an active wet sea point (1 or 2); false otherwise.
 */
inline bool isWetPoint(int val) noexcept { return (val > 0 && val != 3); }

// --- countSeaPointsInRect ---------------------------------------------------
/**
 * @brief Counts active sea points in a rectangular bounding box.
 * @param x_min Minimum longitude index (0-based).
 * @param x_max Maximum longitude index (0-based).
 * @param y_min Minimum latitude index (0-based).
 * @param y_max Maximum latitude index (0-based).
 * @param nx Number of longitudinal grid columns.
 * @param land_sea_map 2D land-sea mask span.
 * @return Count of sea points (where cell is wet and not excluded).
 */
int countSeaPointsInRect(int x_min, int x_max, int y_min, int y_max, int nx,
                         std::span<const int> land_sea_map) {
  int count = 0;
  for (int y = y_min; y <= y_max; ++y) {
    for (int x = x_min; x <= x_max; ++x) {
      if (isWetPoint(land_sea_map[y * nx + x])) {
        count++;
      }
    }
  }
  return count;
}

// --- recursiveBisectDomain --------------------------------------------------
/**
 * @brief Recursively bisects a domain bounding box into target sub-partitions.
 * @param x_min Minimum longitude index (0-based).
 * @param x_max Maximum longitude index (0-based).
 * @param y_min Minimum latitude index (0-based).
 * @param y_max Maximum latitude index (0-based).
 * @param nx Number of longitudinal grid columns.
 * @param land_sea_map 2D land-sea mask span.
 * @param target_partitions Number of partitions to divide into.
 * @param partition_start_id Starting partition ID for this sub-block.
 * @param[out] partition_map Output partition map vector to update.
 */
void recursiveBisectDomain(int x_min, int x_max, int y_min, int y_max, int nx,
                           std::span<const int> land_sea_map,
                           int target_partitions, int partition_start_id,
                           std::vector<int> &partition_map) {
  int total_sea =
      countSeaPointsInRect(x_min, x_max, y_min, y_max, nx, land_sea_map);
  if (total_sea == 0) {
    return;
  }

  if (target_partitions <= 1) {
    for (int y = y_min; y <= y_max; ++y) {
      for (int x = x_min; x <= x_max; ++x) {
        int idx = y * nx + x;
        if (isWetPoint(land_sea_map[idx])) {
          partition_map[idx] = partition_start_id;
        }
      }
    }
    return;
  }

  int p1 = target_partitions / 2;
  int p2 = target_partitions - p1;

  int target_s1 = static_cast<int>(
      std::round(static_cast<double>(total_sea) * static_cast<double>(p1) /
                 static_cast<double>(target_partitions)));
  target_s1 = std::clamp(target_s1, 1, total_sea - 1);

  bool split_x = (x_max - x_min >= y_max - y_min);

  if (split_x && x_max > x_min) {
    int current_sea = 0;
    int best_x = x_min;
    int best_diff = total_sea + 1;

    for (int x = x_min; x < x_max; ++x) {
      int sea_in_col =
          countSeaPointsInRect(x, x, y_min, y_max, nx, land_sea_map);
      current_sea += sea_in_col;
      int diff = std::abs(current_sea - target_s1);
      if (diff < best_diff) {
        best_diff = diff;
        best_x = x;
      }
    }

    recursiveBisectDomain(x_min, best_x, y_min, y_max, nx, land_sea_map, p1,
                          partition_start_id, partition_map);
    recursiveBisectDomain(best_x + 1, x_max, y_min, y_max, nx, land_sea_map, p2,
                          partition_start_id + p1, partition_map);
  } else if (y_max > y_min) {
    int current_sea = 0;
    int best_y = y_min;
    int best_diff = total_sea + 1;

    for (int y = y_min; y < y_max; ++y) {
      int sea_in_row =
          countSeaPointsInRect(x_min, x_max, y, y, nx, land_sea_map);
      current_sea += sea_in_row;
      int diff = std::abs(current_sea - target_s1);
      if (diff < best_diff) {
        best_diff = diff;
        best_y = y;
      }
    }

    recursiveBisectDomain(x_min, x_max, y_min, best_y, nx, land_sea_map, p1,
                          partition_start_id, partition_map);
    recursiveBisectDomain(x_min, x_max, best_y + 1, y_max, nx, land_sea_map, p2,
                          partition_start_id + p1, partition_map);
  } else {
    for (int y = y_min; y <= y_max; ++y) {
      for (int x = x_min; x <= x_max; ++x) {
        int idx = y * nx + x;
        if (isWetPoint(land_sea_map[idx])) {
          partition_map[idx] = partition_start_id;
        }
      }
    }
  }
}

// --- determineDomainNeighbors -----------------------------------------------
/**
 * @brief Identifies 4-connected spatial neighbors for each domain.
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param partition_map Current partition ID assignments.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @return Vector of neighbor ID sets for each partition ID (1-based index).
 */
std::vector<std::set<int>> determineDomainNeighbors(
    int nx, int ny, std::span<const int> land_sea_map, int num_partitions,
    const std::vector<int> &partition_map, bool is_global = false) {
  std::vector<std::set<int>> partition_neighbors(num_partitions + 1);
  if (num_partitions <= 1 || nx <= 0 || ny <= 0) {
    return partition_neighbors;
  }

  const int dx[] = {0, 0, -1, 1};
  const int dy[] = {-1, 1, 0, 0};

  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      int idx = y * nx + x;
      if (!isWetPoint(land_sea_map[idx])) {
        continue;
      }
      int p1 = partition_map[idx];
      if (p1 < 1 || p1 > num_partitions) {
        continue;
      }

      for (int k = 0; k < 4; ++k) {
        int nbr_x = x + dx[k];
        int nbr_y = y + dy[k];
        if (nbr_y < 0 || nbr_y >= ny) {
          continue;
        }
        if (is_global) {
          nbr_x = (nbr_x + nx) % nx;
        } else if (nbr_x < 0 || nbr_x >= nx) {
          continue;
        }

        int nbr_idx = nbr_y * nx + nbr_x;
        if (isWetPoint(land_sea_map[nbr_idx])) {
          int p2 = partition_map[nbr_idx];
          if (p2 >= 1 && p2 <= num_partitions && p2 != p1) {
            partition_neighbors[p1].insert(p2);
          }
        }
      }
    }
  }

  return partition_neighbors;
}

// --- printDomainStats -------------------------------------------------------
/**
 * @brief Prints diagnostic domain statistics to stdout.
 * @details Outputs highest and lowest grid point counts per partition, and
 * the size of the largest partition as a percentage of the target domain size.
 * @param min_points Lowest number of grid points in a partition.
 * @param max_points Highest number of grid points in a partition.
 * @param largest_pct Percentage of largest partition relative to target.
 * @param target_size Target domain size (total wet points / num_partitions,
 * rounded up).
 */
void printDomainStats(int min_points, int max_points, double largest_pct,
                      int target_size) {
  std::cout << "Domain partition stats: Highest = " << max_points
            << ", Lowest = " << min_points << ", Largest = " << std::fixed
            << std::setprecision(1) << largest_pct << "% of target ("
            << target_size << ")" << std::endl;
}

// --- resortDomainsBySize ----------------------------------------------------
/**
 * @brief Re-sorts domains by active wet cell count in descending order.
 * @details Remaps partition IDs 1..num_partitions such that domain 1 has the
 * highest number of wet points and domain num_partitions has the lowest.
 * Reassigns cell values in partition_map in place and recalculates neighbor
 * connectivity.
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Vector of partition assignments.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @param[out] partition_counts Vector of cell counts per sorted partition ID.
 * @param[out] partition_neighbors Vector of neighbor sets per sorted partition
 * ID.
 */
void resortDomainsBySize(int nx, int ny, std::span<const int> land_sea_map,
                         int num_partitions, std::vector<int> &partition_map,
                         bool is_global, std::vector<int> &partition_counts,
                         std::vector<std::set<int>> &partition_neighbors) {
  partition_counts.assign(num_partitions + 1, 0);
  for (size_t i = 0; i < partition_map.size(); ++i) {
    int p = partition_map[i];
    if (isWetPoint(land_sea_map[i]) && p >= 1 && p <= num_partitions) {
      partition_counts[p]++;
    }
  }

  std::vector<std::pair<int, int>> domain_list;
  domain_list.reserve(num_partitions);
  for (int p = 1; p <= num_partitions; ++p) {
    domain_list.emplace_back(partition_counts[p], p);
  }

  std::sort(domain_list.begin(), domain_list.end(),
            [](const std::pair<int, int> &a, const std::pair<int, int> &b) {
              if (a.first != b.first) {
                return a.first > b.first;
              }
              return a.second < b.second;
            });

  std::vector<int> old_to_new(num_partitions + 1, 0);
  std::vector<int> sorted_counts(num_partitions + 1, 0);
  for (int r = 0; r < num_partitions; ++r) {
    int new_id = r + 1;
    int old_id = domain_list[r].second;
    old_to_new[old_id] = new_id;
    sorted_counts[new_id] = domain_list[r].first;
  }

  for (size_t i = 0; i < partition_map.size(); ++i) {
    int old_id = partition_map[i];
    if (old_id >= 1 && old_id <= num_partitions) {
      partition_map[i] = old_to_new[old_id];
    }
  }

  partition_counts = std::move(sorted_counts);
  partition_neighbors = determineDomainNeighbors(
      nx, ny, land_sea_map, num_partitions, partition_map, is_global);
}

// --- reassignDisconnectedPieces ---------------------------------------------
/**
 * @brief Reassigns small disconnected partition pieces to their smallest
 * neighboring partition.
 * @details Identifies connected components for each partition using
 * 4-connectivity (north, south, west, east). For partitions with multiple
 * spatial components, the largest component is designated as the primary bulk
 * of the partition. Small disconnected pieces that share spatial boundaries
 * with neighboring partitions are reassigned to the neighboring partition with
 * the smallest total active sea point count.
 *
 * Algorithm and Connected Component Analysis:
 * Connected Component Identification:
 *   For each partition ID (1 to num_partitions), a 4-connected breadth-first
 *   search (BFS) identifies all spatially contiguous components of active sea
 *   cells assigned to that partition.
 *
 * Bulk Determination and Small Piece Isolation:
 *   If a partition consists of multiple disconnected components, the component
 *   with the largest sea cell count is designated as the partition bulk. All
 *   other components are treated as disconnected small pieces.
 *
 * Neighbor Partition Evaluation and Reassignment:
 *   For each small disconnected piece, the routine scans 4-connected spatial
 *   neighbors of all cells in the piece to identify adjacent partitions.
 *   If adjacent neighboring partitions exist, the small piece is reassigned to
 *   the neighbor with the minimum active sea point count. Partition counts are
 *   updated dynamically after each reassignment.
 *
 * Sweep Convergence:
 *   Processing iterates across all partitions until a complete pass results in
 *   zero disconnected piece reassignments or an iteration safety cap is
 *   reached.
 *
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Partition IDs vector to update in place.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @return Total number of disconnected piece reassignments executed.
 */
int reassignDisconnectedPieces(int nx, int ny,
                               std::span<const int> land_sea_map,
                               int num_partitions,
                               std::vector<int> &partition_map,
                               bool is_global = false) {
  if (num_partitions <= 1 || nx <= 0 || ny <= 0) {
    return 0;
  }

  // === Section: Spatial Connectivity Direction Vectors ======================
  // Define 4-connected spatial neighbor direction vectors (south, north, west,
  // east)
  const int dx[] = {0, 0, -1, 1};
  const int dy[] = {-1, 1, 0, 0};

  int total_reassignments = 0;
  const int max_passes = 50;

  // === Section: Component Analysis and Reassignment Loop ====================
  for (int pass = 0; pass < max_passes; ++pass) {
    int reassignments_in_pass = 0;

    // --- Partition Sea Point Inventory --------------------------------------
    // Count current active sea points assigned to each partition ID
    std::vector<int> partition_counts(num_partitions + 1, 0);
    for (size_t i = 0; i < partition_map.size(); ++i) {
      int p = partition_map[i];
      if (isWetPoint(land_sea_map[i]) && p >= 1 && p <= num_partitions) {
        partition_counts[p]++;
      }
    }

    // --- Partition Component Inspection Sweep -------------------------------
    for (int p = 1; p <= num_partitions; ++p) {
      if (partition_counts[p] == 0) {
        continue;
      }

      // Execute BFS to identify all 4-connected components for partition p
      std::vector<bool> visited(nx * ny, false);
      std::vector<std::vector<int>> components;

      for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx; ++x) {
          int idx = y * nx + x;
          if (!isWetPoint(land_sea_map[idx]) || partition_map[idx] != p ||
              visited[idx]) {
            continue;
          }

          std::vector<int> comp;
          std::vector<int> queue;
          queue.push_back(idx);
          visited[idx] = true;

          size_t head = 0;
          while (head < queue.size()) {
            int curr = queue[head++];
            comp.push_back(curr);
            int cx = curr % nx;
            int cy = curr / nx;

            for (int k = 0; k < 4; ++k) {
              int nbr_x = cx + dx[k];
              int nbr_y = cy + dy[k];
              if (nbr_y < 0 || nbr_y >= ny) {
                continue;
              }
              if (is_global) {
                nbr_x = (nbr_x + nx) % nx;
              } else if (nbr_x < 0 || nbr_x >= nx) {
                continue;
              }
              int nbr_idx = nbr_y * nx + nbr_x;
              if (isWetPoint(land_sea_map[nbr_idx]) &&
                  partition_map[nbr_idx] == p && !visited[nbr_idx]) {
                visited[nbr_idx] = true;
                queue.push_back(nbr_idx);
              }
            }
          }
          components.push_back(comp);
        }
      }

      // Single component partitions are fully connected; skip reassignment
      if (components.size() <= 1) {
        continue;
      }

      // --- Bulk Identification and Small Piece Resolution -------------------
      // Locate the largest component to designate as the partition bulk
      size_t bulk_idx = 0;
      size_t max_size = 0;
      for (size_t i = 0; i < components.size(); ++i) {
        if (components[i].size() > max_size) {
          max_size = components[i].size();
          bulk_idx = i;
        }
      }

      // Reassign disconnected small components (all components except bulk)
      for (size_t i = 0; i < components.size(); ++i) {
        if (i == bulk_idx) {
          continue;
        }

        const auto &small_piece = components[i];

        // Gather all external 4-connected neighbor partition IDs
        std::set<int> neighbor_partitions;
        for (int cell_idx : small_piece) {
          int cx = cell_idx % nx;
          int cy = cell_idx / nx;

          for (int k = 0; k < 4; ++k) {
            int nbr_x = cx + dx[k];
            int nbr_y = cy + dy[k];
            if (nbr_y < 0 || nbr_y >= ny) {
              continue;
            }
            if (is_global) {
              nbr_x = (nbr_x + nx) % nx;
            } else if (nbr_x < 0 || nbr_x >= nx) {
              continue;
            }
            int nbr_idx = nbr_y * nx + nbr_x;
            if (isWetPoint(land_sea_map[nbr_idx])) {
              int nbr_p = partition_map[nbr_idx];
              if (nbr_p >= 1 && nbr_p <= num_partitions && nbr_p != p) {
                neighbor_partitions.insert(nbr_p);
              }
            }
          }
        }

        // Reassign cells in small piece to the smallest adjacent neighbor
        if (!neighbor_partitions.empty()) {
          int smallest_nbr = -1;
          int min_count = -1;

          for (int nbr_p : neighbor_partitions) {
            int count = partition_counts[nbr_p];
            if (smallest_nbr == -1 || count < min_count ||
                (count == min_count && nbr_p < smallest_nbr)) {
              min_count = count;
              smallest_nbr = nbr_p;
            }
          }

          if (smallest_nbr != -1) {
            for (int cell_idx : small_piece) {
              partition_map[cell_idx] = smallest_nbr;
            }
            int piece_size = static_cast<int>(small_piece.size());
            partition_counts[p] -= piece_size;
            partition_counts[smallest_nbr] += piece_size;
            reassignments_in_pass++;
            total_reassignments++;
          }
        }
      }
    }

    // --- Pass Termination Check --------------------------------------------
    if (reassignments_in_pass == 0) {
      break;
    }
  }

  return total_reassignments;
}

// --- dealWithSmallGridExtensions --------------------------------------------
/**
 * @brief Placeholder subroutine to deal with small grid extensions.
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Partition assignments vector.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @return Number of cell adjustments executed.
 */
int dealWithSmallGridExtensions(
    [[maybe_unused]] int nx, [[maybe_unused]] int ny,
    [[maybe_unused]] std::span<const int> land_sea_map,
    [[maybe_unused]] int num_partitions,
    [[maybe_unused]] const std::vector<int> &partition_map,
    [[maybe_unused]] bool is_global = false) {
  // Placeholder for now
  return 0;
}

// --- rebalanceIsolatedSmallestDomain ----------------------------------------
/**
 * @brief Rebalances an isolated smallest domain (< 70% target) by transferring
 * a coherent part from the largest domain that has neighbors.
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Partition IDs vector to update.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @param[in,out] partition_counts Vector of wet cell counts per partition.
 * @param[in,out] partition_neighbors Vector of neighbor sets per partition.
 * @param target_size Target domain size (ceil(total_wet / num_partitions)).
 * @return True if a transfer was performed; false otherwise.
 */
bool rebalanceIsolatedSmallestDomain(
    int nx, int ny, std::span<const int> land_sea_map, int num_partitions,
    std::vector<int> &partition_map, bool is_global,
    std::vector<int> &partition_counts,
    std::vector<std::set<int>> &partition_neighbors, int target_size) {
  if (num_partitions <= 1) {
    return false;
  }

  int smallest_p = num_partitions;
  if (!partition_neighbors[smallest_p].empty()) {
    return false;
  }

  double threshold = 0.70 * static_cast<double>(target_size);
  if (static_cast<double>(partition_counts[smallest_p]) >= threshold) {
    return false;
  }

  // Find largest domain that has neighbors
  int largest_with_nbrs = -1;
  for (int p = 1; p <= num_partitions; ++p) {
    if (!partition_neighbors[p].empty() && partition_counts[p] > 0) {
      largest_with_nbrs = p;
      break;
    }
  }

  if (largest_with_nbrs == -1) {
    return false;
  }

  int needed = target_size - partition_counts[smallest_p];
  if (needed <= 0) {
    return false;
  }

  int max_take = partition_counts[largest_with_nbrs] - 1;
  if (max_take <= 0) {
    return false;
  }
  int points_to_transfer = std::min(needed, max_take);

  // Extract a coherent part of largest_with_nbrs using BFS
  int start_cell = -1;
  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      int idx = y * nx + x;
      if (isWetPoint(land_sea_map[idx]) &&
          partition_map[idx] == largest_with_nbrs) {
        start_cell = idx;
        break;
      }
    }
    if (start_cell != -1) {
      break;
    }
  }

  if (start_cell == -1) {
    return false;
  }

  const int dx[] = {0, 0, -1, 1};
  const int dy[] = {-1, 1, 0, 0};

  std::vector<int> transferred_cells;
  transferred_cells.reserve(points_to_transfer);

  std::vector<bool> visited(nx * ny, false);
  std::vector<int> queue;
  queue.push_back(start_cell);
  visited[start_cell] = true;

  size_t head = 0;
  while (head < queue.size() &&
         static_cast<int>(transferred_cells.size()) < points_to_transfer) {
    int curr = queue[head++];
    transferred_cells.push_back(curr);

    int cx = curr % nx;
    int cy = curr / nx;

    for (int k = 0; k < 4; ++k) {
      int nbr_x = cx + dx[k];
      int nbr_y = cy + dy[k];
      if (nbr_y < 0 || nbr_y >= ny) {
        continue;
      }
      if (is_global) {
        nbr_x = (nbr_x + nx) % nx;
      } else if (nbr_x < 0 || nbr_x >= nx) {
        continue;
      }
      int nbr_idx = nbr_y * nx + nbr_x;
      if (isWetPoint(land_sea_map[nbr_idx]) &&
          partition_map[nbr_idx] == largest_with_nbrs && !visited[nbr_idx]) {
        visited[nbr_idx] = true;
        queue.push_back(nbr_idx);
      }
    }
  }

  if (transferred_cells.empty()) {
    return false;
  }

  for (int idx : transferred_cells) {
    partition_map[idx] = smallest_p;
  }

  resortDomainsBySize(nx, ny, land_sea_map, num_partitions, partition_map,
                      is_global, partition_counts, partition_neighbors);
  return true;
}

// --- splitAndMergeLargestIsolatedDomain -------------------------------------
/**
 * @brief Splits an isolated largest domain into two near-equal parts and merges
 * the smallest adjacent pair of domains that have additional neighbors.
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Partition IDs vector to update.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @param[in,out] partition_counts Vector of wet cell counts per partition.
 * @param[in,out] partition_neighbors Vector of neighbor sets per partition.
 * @return True if split and merge occurred; false otherwise.
 */
bool splitAndMergeLargestIsolatedDomain(
    int nx, int ny, std::span<const int> land_sea_map, int num_partitions,
    std::vector<int> &partition_map, bool is_global,
    std::vector<int> &partition_counts,
    std::vector<std::set<int>> &partition_neighbors) {
  if (num_partitions <= 1) {
    return false;
  }

  int largest_p = 1;
  if (!partition_neighbors[largest_p].empty()) {
    return false;
  }

  // Find candidate adjacent pair with smallest combined count and additional
  // neighbors
  int best_q1 = -1;
  int best_q2 = -1;
  int min_pair_sum = -1;

  for (int q1 = 1; q1 <= num_partitions; ++q1) {
    if (q1 == largest_p || partition_counts[q1] == 0) {
      continue;
    }
    for (int q2 : partition_neighbors[q1]) {
      if (q2 <= q1 || q2 == largest_p || partition_counts[q2] == 0) {
        continue;
      }

      auto is_external_nbr = [q1, q2, largest_p](int nbr) {
        return nbr != q1 && nbr != q2 && nbr != largest_p;
      };

      bool has_external_neighbor =
          std::any_of(partition_neighbors[q1].begin(),
                      partition_neighbors[q1].end(), is_external_nbr) ||
          std::any_of(partition_neighbors[q2].begin(),
                      partition_neighbors[q2].end(), is_external_nbr);

      if (!has_external_neighbor) {
        continue;
      }

      int pair_sum = partition_counts[q1] + partition_counts[q2];
      if (min_pair_sum == -1 || pair_sum < min_pair_sum) {
        min_pair_sum = pair_sum;
        best_q1 = q1;
        best_q2 = q2;
      }
    }
  }

  if (best_q1 == -1 || best_q2 == -1) {
    return false;
  }

  int q_keep = best_q1;
  int q_released = best_q2;

  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (isWetPoint(land_sea_map[i]) && partition_map[i] == q_released) {
      partition_map[i] = q_keep;
    }
  }

  int x_min = nx, x_max = -1, y_min = ny, y_max = -1;
  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      int idx = y * nx + x;
      if (isWetPoint(land_sea_map[idx]) && partition_map[idx] == largest_p) {
        x_min = std::min(x_min, x);
        x_max = std::max(x_max, x);
        y_min = std::min(y_min, y);
        y_max = std::max(y_max, y);
      }
    }
  }

  int isolated_sea = partition_counts[largest_p];
  int target_half = isolated_sea / 2;

  bool split_x = (x_max - x_min >= y_max - y_min);

  if (split_x && x_max > x_min) {
    int current_sea = 0;
    int best_x = x_min;
    int best_diff = isolated_sea + 1;

    for (int x = x_min; x < x_max; ++x) {
      int sea_in_col = 0;
      for (int y = y_min; y <= y_max; ++y) {
        int idx = y * nx + x;
        if (isWetPoint(land_sea_map[idx]) && partition_map[idx] == largest_p) {
          sea_in_col++;
        }
      }
      current_sea += sea_in_col;
      int diff = std::abs(current_sea - target_half);
      if (diff < best_diff) {
        best_diff = diff;
        best_x = x;
      }
    }

    for (int y = y_min; y <= y_max; ++y) {
      for (int x = best_x + 1; x <= x_max; ++x) {
        int idx = y * nx + x;
        if (isWetPoint(land_sea_map[idx]) && partition_map[idx] == largest_p) {
          partition_map[idx] = q_released;
        }
      }
    }
  } else if (y_max > y_min) {
    int current_sea = 0;
    int best_y = y_min;
    int best_diff = isolated_sea + 1;

    for (int y = y_min; y < y_max; ++y) {
      int sea_in_row = 0;
      for (int x = x_min; x <= x_max; ++x) {
        int idx = y * nx + x;
        if (isWetPoint(land_sea_map[idx]) && partition_map[idx] == largest_p) {
          sea_in_row++;
        }
      }
      current_sea += sea_in_row;
      int diff = std::abs(current_sea - target_half);
      if (diff < best_diff) {
        best_diff = diff;
        best_y = y;
      }
    }

    for (int y = best_y + 1; y <= y_max; ++y) {
      for (int x = x_min; x <= x_max; ++x) {
        int idx = y * nx + x;
        if (isWetPoint(land_sea_map[idx]) && partition_map[idx] == largest_p) {
          partition_map[idx] = q_released;
        }
      }
    }
  }

  resortDomainsBySize(nx, ny, land_sea_map, num_partitions, partition_map,
                      is_global, partition_counts, partition_neighbors);
  return true;
}

// --- extractBoundaryLine ----------------------------------------------------
/**
 * @brief Extracts a contiguous line (segment along row or column) of boundary
 * cells belonging to src domain that border dst domain.
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param partition_map Current partition assignments.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @param src Source partition ID.
 * @param dst Destination partition ID.
 * @return Vector of cell indices forming the contiguous boundary line.
 */
std::vector<int> extractBoundaryLine(int nx, int ny,
                                     std::span<const int> land_sea_map,
                                     const std::vector<int> &partition_map,
                                     bool is_global, int src, int dst) {
  const int dx[] = {0, 0, -1, 1};
  const int dy[] = {-1, 1, 0, 0};

  std::vector<int> longest_line;
  std::vector<bool> checked_h(nx * ny, false);
  std::vector<bool> checked_v(nx * ny, false);

  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      int idx = y * nx + x;
      if (!isWetPoint(land_sea_map[idx]) || partition_map[idx] != src) {
        continue;
      }

      for (int k = 0; k < 4; ++k) {
        if ((k == 0 || k == 1) && checked_h[idx]) {
          continue;
        }
        if ((k == 2 || k == 3) && checked_v[idx]) {
          continue;
        }

        int nbr_x = x + dx[k];
        int nbr_y = y + dy[k];
        if (nbr_y < 0 || nbr_y >= ny) {
          continue;
        }
        if (is_global) {
          nbr_x = (nbr_x + nx) % nx;
        } else if (nbr_x < 0 || nbr_x >= nx) {
          continue;
        }

        int nbr_idx = nbr_y * nx + nbr_x;
        if (isWetPoint(land_sea_map[nbr_idx]) &&
            partition_map[nbr_idx] == dst) {
          std::vector<int> line_indices;
          if (k == 0 || k == 1) {
            // Horizontal line along row y
            int left_count = 0;
            int cur_left = x;
            while (left_count < nx - 1) {
              int next_left =
                  is_global ? ((cur_left - 1 + nx) % nx) : (cur_left - 1);
              if (next_left < 0) {
                break;
              }
              int test_idx = y * nx + next_left;
              if (!isWetPoint(land_sea_map[test_idx]) ||
                  partition_map[test_idx] != src) {
                break;
              }
              int test_nbr_x = next_left + dx[k];
              int test_nbr_y = y + dy[k];
              if (test_nbr_y < 0 || test_nbr_y >= ny) {
                break;
              }
              if (is_global) {
                test_nbr_x = (test_nbr_x + nx) % nx;
              }
              int test_nbr_idx = test_nbr_y * nx + test_nbr_x;
              if (!isWetPoint(land_sea_map[test_nbr_idx]) ||
                  partition_map[test_nbr_idx] != dst) {
                break;
              }
              cur_left = next_left;
              left_count++;
            }

            int right_count = 0;
            int cur_right = x;
            while (left_count + right_count < nx - 1) {
              int next_right =
                  is_global ? ((cur_right + 1) % nx) : (cur_right + 1);
              if (next_right >= nx) {
                break;
              }
              int test_idx = y * nx + next_right;
              if (!isWetPoint(land_sea_map[test_idx]) ||
                  partition_map[test_idx] != src) {
                break;
              }
              int test_nbr_x = next_right + dx[k];
              int test_nbr_y = y + dy[k];
              if (test_nbr_y < 0 || test_nbr_y >= ny) {
                break;
              }
              if (is_global) {
                test_nbr_x = (test_nbr_x + nx) % nx;
              }
              int test_nbr_idx = test_nbr_y * nx + test_nbr_x;
              if (!isWetPoint(land_sea_map[test_nbr_idx]) ||
                  partition_map[test_nbr_idx] != dst) {
                break;
              }
              cur_right = next_right;
              right_count++;
            }

            int total_line = left_count + 1 + right_count;
            int cur = cur_left;
            for (int s = 0; s < total_line; ++s) {
              int line_cell = y * nx + cur;
              line_indices.push_back(line_cell);
              checked_h[line_cell] = true;
              cur = is_global ? ((cur + 1) % nx) : (cur + 1);
            }
          } else {
            // Vertical line along column x
            int y_start = y;
            while (y_start > 0) {
              int test_idx = (y_start - 1) * nx + x;
              if (!isWetPoint(land_sea_map[test_idx]) ||
                  partition_map[test_idx] != src) {
                break;
              }
              int test_nbr_x = x + dx[k];
              int test_nbr_y = (y_start - 1) + dy[k];
              if (is_global) {
                test_nbr_x = (test_nbr_x + nx) % nx;
              } else if (test_nbr_x < 0 || test_nbr_x >= nx) {
                break;
              }
              int test_nbr_idx = test_nbr_y * nx + test_nbr_x;
              if (!isWetPoint(land_sea_map[test_nbr_idx]) ||
                  partition_map[test_nbr_idx] != dst) {
                break;
              }
              y_start--;
            }

            int y_end = y;
            while (y_end < ny - 1) {
              int test_idx = (y_end + 1) * nx + x;
              if (!isWetPoint(land_sea_map[test_idx]) ||
                  partition_map[test_idx] != src) {
                break;
              }
              int test_nbr_x = x + dx[k];
              int test_nbr_y = (y_end + 1) + dy[k];
              if (is_global) {
                test_nbr_x = (test_nbr_x + nx) % nx;
              } else if (test_nbr_x < 0 || test_nbr_x >= nx) {
                break;
              }
              int test_nbr_idx = test_nbr_y * nx + test_nbr_x;
              if (!isWetPoint(land_sea_map[test_nbr_idx]) ||
                  partition_map[test_nbr_idx] != dst) {
                break;
              }
              y_end++;
            }

            for (int r = y_start; r <= y_end; ++r) {
              int line_cell = r * nx + x;
              line_indices.push_back(line_cell);
              checked_v[line_cell] = true;
            }
          }

          if (line_indices.size() > longest_line.size()) {
            longest_line = std::move(line_indices);
          }
        }
      }
    }
  }

  return longest_line;
}

// --- exchangeRankedBoundaries -----------------------------------------------
/**
 * @brief Exchanges boundary lines between size-ranked domains and their
 * neighbors.
 * @details Iterates from largest to smallest domain. If above target,
 * transfers grid points per line to its smallest neighbor. If below target,
 * takes a line of cells from its largest neighbor. Adjusts cell counts on the
 * fly without re-sorting during the loop. Rejects any exchange if the domain
 * furthest from target becomes even further away in absolute value.
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Partition assignments vector.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @param[in,out] partition_counts Vector of wet cell counts per partition.
 * @param target_size Target domain size (ceil(total_wet / num_partitions)).
 * @return Total number of boundary cells swapped.
 */
int exchangeRankedBoundaries(int nx, int ny, std::span<const int> land_sea_map,
                             int num_partitions,
                             std::vector<int> &partition_map, bool is_global,
                             std::vector<int> &partition_counts,
                             int target_size) {
  const int dx[] = {0, 0, -1, 1};
  const int dy[] = {-1, 1, 0, 0};

  int total_swaps = 0;

  for (int p = 1; p <= num_partitions; ++p) {
    std::set<int> neighbors;
    for (int y = 0; y < ny; ++y) {
      for (int x = 0; x < nx; ++x) {
        int idx = y * nx + x;
        if (!isWetPoint(land_sea_map[idx]) || partition_map[idx] != p) {
          continue;
        }
        for (int k = 0; k < 4; ++k) {
          int nbr_x = x + dx[k];
          int nbr_y = y + dy[k];
          if (nbr_y < 0 || nbr_y >= ny) {
            continue;
          }
          if (is_global) {
            nbr_x = (nbr_x + nx) % nx;
          } else if (nbr_x < 0 || nbr_x >= nx) {
            continue;
          }
          int nbr_idx = nbr_y * nx + nbr_x;
          if (isWetPoint(land_sea_map[nbr_idx])) {
            int nbr_p = partition_map[nbr_idx];
            if (nbr_p >= 1 && nbr_p <= num_partitions && nbr_p != p) {
              neighbors.insert(nbr_p);
            }
          }
        }
      }
    }

    if (neighbors.empty()) {
      continue;
    }

    if (partition_counts[p] > target_size) {
      int smallest_nbr = -1;
      int min_count = -1;
      for (int nbr : neighbors) {
        if (smallest_nbr == -1 || partition_counts[nbr] < min_count) {
          min_count = partition_counts[nbr];
          smallest_nbr = nbr;
        }
      }

      if (smallest_nbr != -1) {
        std::vector<int> line = extractBoundaryLine(
            nx, ny, land_sea_map, partition_map, is_global, p, smallest_nbr);
        int S = static_cast<int>(line.size());
        if (S > 0 && partition_counts[p] > S) {
          int dev_before =
              std::max(std::abs(partition_counts[p] - target_size),
                       std::abs(partition_counts[smallest_nbr] - target_size));
          int new_p = partition_counts[p] - S;
          int new_nbr = partition_counts[smallest_nbr] + S;
          int dev_after = std::max(std::abs(new_p - target_size),
                                   std::abs(new_nbr - target_size));

          if (dev_after <= dev_before) {
            for (int idx : line) {
              partition_map[idx] = smallest_nbr;
            }
            partition_counts[p] = new_p;
            partition_counts[smallest_nbr] = new_nbr;
            total_swaps += S;
          }
        }
      }
    } else if (partition_counts[p] < target_size) {
      int largest_nbr = -1;
      int max_count = -1;
      for (int nbr : neighbors) {
        if (largest_nbr == -1 || partition_counts[nbr] > max_count) {
          max_count = partition_counts[nbr];
          largest_nbr = nbr;
        }
      }

      if (largest_nbr != -1) {
        std::vector<int> line = extractBoundaryLine(
            nx, ny, land_sea_map, partition_map, is_global, largest_nbr, p);
        int S = static_cast<int>(line.size());
        if (S > 0 && partition_counts[largest_nbr] > S) {
          int dev_before =
              std::max(std::abs(partition_counts[p] - target_size),
                       std::abs(partition_counts[largest_nbr] - target_size));
          int new_p = partition_counts[p] + S;
          int new_nbr = partition_counts[largest_nbr] - S;
          int dev_after = std::max(std::abs(new_p - target_size),
                                   std::abs(new_nbr - target_size));

          if (dev_after <= dev_before) {
            for (int idx : line) {
              partition_map[idx] = p;
            }
            partition_counts[p] = new_p;
            partition_counts[largest_nbr] = new_nbr;
            total_swaps += S;
          }
        }
      }
    }
  }

  return total_swaps;
}

// --- cleanSingleProtrudingPoints --------------------------------------------
/**
 * @brief Checks outer boundaries of partitions for narrow 1-point or 2-point
 * wide lines of grid points jutting out from partition bodies and reassigns
 * them to adjacent partitions if possible.
 * @details Detects narrow 1-point or 2-point wide peninsulas / lines jutting
 * out from the body of a partition into adjacent partitions. Reassigning such
 * points to adjacent partitions smooths boundaries and prevents jagged lines
 * without increasing partition imbalance when feasible.
 *
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Partition IDs vector to update in place.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @return Total number of protruding points reassigned.
 */
[[maybe_unused]] int
cleanSingleProtrudingPoints(int nx, int ny, std::span<const int> land_sea_map,
                            int num_partitions, std::vector<int> &partition_map,
                            bool is_global = false) {
  if (num_partitions <= 1 || nx <= 0 || ny <= 0) {
    return 0;
  }

  const int dx[] = {0, 0, -1, 1};
  const int dy[] = {-1, 1, 0, 0};

  // Count active points per partition
  std::vector<int> partition_counts(num_partitions + 1, 0);
  for (size_t i = 0; i < partition_map.size(); ++i) {
    int p = partition_map[i];
    if (isWetPoint(land_sea_map[i]) && p >= 1 && p <= num_partitions) {
      partition_counts[p]++;
    }
  }

  int total_cleaned = 0;
  const int max_passes = 10;

  for (int pass = 0; pass < max_passes; ++pass) {
    int cleaned_in_pass = 0;

    // Check 1: single protruding points
    for (int y = 0; y < ny; ++y) {
      for (int x = 0; x < nx; ++x) {
        int idx = y * nx + x;
        if (!isWetPoint(land_sea_map[idx])) {
          continue;
        }

        int p1 = partition_map[idx];
        if (p1 < 1 || p1 > num_partitions) {
          continue;
        }

        // Must not leave a partition with zero points
        if (partition_counts[p1] <= 1) {
          continue;
        }

        int count_same = 0;
        int cand_p[4] = {0, 0, 0, 0};
        int cand_c[4] = {0, 0, 0, 0};
        int num_cands = 0;

        for (int k = 0; k < 4; ++k) {
          int nbr_x = x + dx[k];
          int nbr_y = y + dy[k];

          if (nbr_y < 0 || nbr_y >= ny) {
            continue;
          }
          if (is_global) {
            nbr_x = (nbr_x + nx) % nx;
          } else if (nbr_x < 0 || nbr_x >= nx) {
            continue;
          }

          int nbr_idx = nbr_y * nx + nbr_x;
          if (!isWetPoint(land_sea_map[nbr_idx])) {
            continue;
          }

          int p2 = partition_map[nbr_idx];
          if (p2 == p1) {
            count_same++;
          } else if (p2 >= 1 && p2 <= num_partitions) {
            bool found = false;
            for (int c = 0; c < num_cands; ++c) {
              if (cand_p[c] == p2) {
                cand_c[c]++;
                found = true;
                break;
              }
            }
            if (!found) {
              cand_p[num_cands] = p2;
              cand_c[num_cands] = 1;
              num_cands++;
            }
          }
        }

        // A single protruding point has at most 1 neighbor in its own partition
        // and at least 2 neighbors in an adjacent partition (or total neighbors
        // predominantly belonging to other partitions)
        if (num_cands > 0 && count_same <= 1) {
          // Find the best adjacent candidate partition
          int best_p2 = -1;
          int max_nbrs = 0;
          for (int c = 0; c < num_cands; ++c) {
            if (cand_c[c] > max_nbrs) {
              max_nbrs = cand_c[c];
              best_p2 = cand_p[c];
            } else if (cand_c[c] == max_nbrs && best_p2 != -1) {
              if (partition_counts[cand_p[c]] < partition_counts[best_p2]) {
                best_p2 = cand_p[c];
              }
            }
          }

          if (best_p2 != -1 && max_nbrs >= 2) {
            // Check imbalance condition: do not increase imbalance
            int diff_before =
                std::abs(partition_counts[p1] - partition_counts[best_p2]);
            int diff_after = std::abs((partition_counts[p1] - 1) -
                                      (partition_counts[best_p2] + 1));
            if (diff_after <= diff_before) {
              partition_map[idx] = best_p2;
              partition_counts[p1]--;
              partition_counts[best_p2]++;
              cleaned_in_pass++;
              total_cleaned++;
            }
          }
        }
      }
    }

    // Check 2: 2-point wide narrow lines protruding into an adjacent partition
    for (int y = 0; y < ny; ++y) {
      for (int x = 0; x < nx; ++x) {
        int idx1 = y * nx + x;
        if (!isWetPoint(land_sea_map[idx1])) {
          continue;
        }
        int p1 = partition_map[idx1];
        if (p1 < 1 || p1 > num_partitions || partition_counts[p1] <= 2) {
          continue;
        }

        // Check horizontal or vertical pair (x, y) and its north or east
        // neighbor
        for (int pair_dir = 1; pair_dir <= 3; pair_dir += 2) {
          int x2 = x + dx[pair_dir];
          int y2 = y + dy[pair_dir];
          if (y2 < 0 || y2 >= ny) {
            continue;
          }
          if (is_global) {
            x2 = (x2 + nx) % nx;
          } else if (x2 < 0 || x2 >= nx) {
            continue;
          }
          int idx2 = y2 * nx + x2;
          if (!isWetPoint(land_sea_map[idx2]) || partition_map[idx2] != p1) {
            continue;
          }

          int nbrs_p1_cell = 0;
          int cand_p_pair[6] = {0, 0, 0, 0, 0, 0};
          int cand_c_pair[6] = {0, 0, 0, 0, 0, 0};
          int num_cands_pair = 0;

          auto add_nbrs = [&](int cx, int cy, int other_idx) {
            for (int k = 0; k < 4; ++k) {
              int nx_nbr = cx + dx[k];
              int ny_nbr = cy + dy[k];
              if (ny_nbr < 0 || ny_nbr >= ny) {
                continue;
              }
              if (is_global) {
                nx_nbr = (nx_nbr + nx) % nx;
              } else if (nx_nbr < 0 || nx_nbr >= nx) {
                continue;
              }
              int nidx = ny_nbr * nx + nx_nbr;
              if (nidx == other_idx || !isWetPoint(land_sea_map[nidx])) {
                continue;
              }
              int p_nbr = partition_map[nidx];
              if (p_nbr == p1) {
                nbrs_p1_cell++;
              } else if (p_nbr >= 1 && p_nbr <= num_partitions) {
                bool found = false;
                for (int c = 0; c < num_cands_pair; ++c) {
                  if (cand_p_pair[c] == p_nbr) {
                    cand_c_pair[c]++;
                    found = true;
                    break;
                  }
                }
                if (!found && num_cands_pair < 6) {
                  cand_p_pair[num_cands_pair] = p_nbr;
                  cand_c_pair[num_cands_pair] = 1;
                  num_cands_pair++;
                }
              }
            }
          };

          nbrs_p1_cell = 0;
          add_nbrs(x, y, idx2);
          int p1_ext_1 = nbrs_p1_cell;

          nbrs_p1_cell = 0;
          add_nbrs(x2, y2, idx1);
          int p1_ext_2 = nbrs_p1_cell;

          if (p1_ext_1 + p1_ext_2 <= 1 && num_cands_pair > 0) {
            int best_p2 = -1;
            int max_c = 0;
            for (int c = 0; c < num_cands_pair; ++c) {
              if (cand_c_pair[c] > max_c) {
                max_c = cand_c_pair[c];
                best_p2 = cand_p_pair[c];
              } else if (cand_c_pair[c] == max_c && best_p2 != -1) {
                if (partition_counts[cand_p_pair[c]] <
                    partition_counts[best_p2]) {
                  best_p2 = cand_p_pair[c];
                }
              }
            }

            if (best_p2 != -1 && max_c >= 3) {
              int diff_before =
                  std::abs(partition_counts[p1] - partition_counts[best_p2]);
              int diff_after = std::abs((partition_counts[p1] - 2) -
                                        (partition_counts[best_p2] + 2));
              if (diff_after <= diff_before) {
                partition_map[idx1] = best_p2;
                partition_map[idx2] = best_p2;
                partition_counts[p1] -= 2;
                partition_counts[best_p2] += 2;
                cleaned_in_pass += 2;
                total_cleaned += 2;
              }
            }
          }
        }
      }
    }

    if (cleaned_in_pass == 0) {
      break;
    }
  }

  return total_cleaned;
}

// --- refinePartitionsIteratively --------------------------------------------
/**
 * @brief Iteratively refines partition assignments at domain boundaries to
 * balance active sea point counts across partitions.
 * @details Examines boundary sea points between overloaded and underloaded
 * adjacent partitions and reassigns points to improve overall load balance
 * until convergence or max_iterations is reached. Point exchanges are executed
 * per row or column of adjacent wet points on the interface (unless adjacent
 * boundary points are not wet). A row or column exchange is rejected if it
 * increases the imbalance between the two partitions involved.
 * Periodic component checks run every 5th iteration to reassign disconnected
 * fragments, and single protruding points are cleaned up.
 *
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of target partitions.
 * @param max_iterations Maximum number of boundary swap passes.
 * @param[in,out] partition_map Partition IDs vector to refine in place.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @param record_hook Callback invoked whenever intermediate maps are updated.
 * @return Number of actual iteration passes executed until convergence or cap.
 */
[[maybe_unused]] int refinePartitionsIteratively(
    int nx, int ny, std::span<const int> land_sea_map, int num_partitions,
    int max_iterations, std::vector<int> &partition_map, bool is_global = false,
    const std::function<void()> &record_hook = nullptr) {
  // Return early if single partition or non-positive max_iterations specified
  if (num_partitions <= 1 || max_iterations <= 0) {
    return 0;
  }

  // === Section: Partition Sea Point Inventory ===============================
  // Count initial active sea points (where land_sea_map > 0) assigned to each
  // partition ID (1 to num_partitions).
  std::vector<int> partition_counts(num_partitions + 1, 0);
  int total_sea = 0;
  for (size_t i = 0; i < partition_map.size(); ++i) {
    int pid = partition_map[i];
    if (isWetPoint(land_sea_map[i]) && pid >= 1 && pid <= num_partitions) {
      partition_counts[pid]++;
      total_sea++;
    }
  }

  // If grid contains no active sea points, return 0 iterations executed
  if (total_sea == 0) {
    return 0;
  }

  // === Section: Target Load Calculation & Spatial Connectivity Setup ========
  double target_per_partition = static_cast<double>(total_sea) / num_partitions;

  const int dx[] = {0, 0, -1, 1};
  const int dy[] = {-1, 1, 0, 0};

  // Helper lambda to get neighbor partition ID (or 0 if invalid/non-wet)
  auto getNeighborPartition = [&](int x, int y, int dir) -> int {
    int nx_nbr = x + dx[dir];
    int ny_nbr = y + dy[dir];
    if (ny_nbr < 0 || ny_nbr >= ny) {
      return 0;
    }
    if (is_global) {
      nx_nbr = (nx_nbr + nx) % nx;
    } else if (nx_nbr < 0 || nx_nbr >= nx) {
      return 0;
    }
    int nbr_idx = ny_nbr * nx + nx_nbr;
    if (!isWetPoint(land_sea_map[nbr_idx])) {
      return 0;
    }
    return partition_map[nbr_idx];
  };

  // === Section: Iterative Refinement Loop ===================================
  int actual_iters = 0;
  for (int iter = 0; iter < max_iterations; ++iter) {
    actual_iters++;
    int swaps_made = 0;

    std::vector<bool> processed_in_iter(nx * ny, false);

    // Execute 2D spatial grid sweep across all rows (y) and columns (x)
    for (int y = 0; y < ny; ++y) {
      for (int x = 0; x < nx; ++x) {
        int idx = y * nx + x;
        if (processed_in_iter[idx] || !isWetPoint(land_sea_map[idx])) {
          continue;
        }

        int p1 = partition_map[idx];
        if (p1 < 1 || p1 > num_partitions) {
          continue;
        }

        // --- Overloaded Partition Filter ------------------------------------
        if (partition_counts[p1] <=
            static_cast<int>(std::floor(target_per_partition))) {
          continue;
        }

        // Find candidate underloaded neighbors
        int cand_p[4] = {0, 0, 0, 0};
        int cand_dir[4] = {-1, -1, -1, -1};
        int num_cands = 0;

        for (int k = 0; k < 4; ++k) {
          int p2 = getNeighborPartition(x, y, k);
          if (p2 >= 1 && p2 <= num_partitions && p2 != p1) {
            if (partition_counts[p2] <
                static_cast<int>(std::ceil(target_per_partition))) {
              cand_p[num_cands] = p2;
              cand_dir[num_cands] = k;
              num_cands++;
            }
          }
        }

        if (num_cands == 0) {
          continue;
        }

        // Prioritize candidate with lowest count
        int best_p2 = cand_p[0];
        int best_dir = cand_dir[0];
        for (int c = 1; c < num_cands; ++c) {
          if (partition_counts[cand_p[c]] < partition_counts[best_p2]) {
            best_p2 = cand_p[c];
            best_dir = cand_dir[c];
          }
        }

        // Identify segment of adjacent wet points on interface:
        // If interface is vertical (neighbor is west k=2 or east k=3), the
        // interface runs vertically along a column of adjacent points sharing
        // that interface. If interface is horizontal (neighbor is south k=0 or
        // north k=1), the interface runs horizontally along a row of adjacent
        // points.
        std::vector<int> segment_indices;

        if (best_dir == 0 || best_dir == 1) {
          // Horizontal interface: inspect adjacent wet points along row y
          // Check if adjacent boundary points along row are wet
          int left_x = is_global ? ((x - 1 + nx) % nx) : (x - 1);
          int right_x = is_global ? ((x + 1) % nx) : (x + 1);
          bool left_wet =
              (left_x >= 0) && isWetPoint(land_sea_map[y * nx + left_x]);
          bool right_wet =
              (right_x < nx) && isWetPoint(land_sea_map[y * nx + right_x]);

          if (!left_wet && !right_wet) {
            // Adjacent points along the boundary row are not wet; treat as
            // individual point
            segment_indices.push_back(idx);
          } else {
            // Collect contiguous segment of adjacent wet points in this row
            // belonging to p1 that border best_p2 in direction best_dir.
            // Account for longitudinal wrapping if is_global is true.
            int left_count = 0;
            int cur_left = x;
            while (left_count < nx - 1) {
              int next_left =
                  is_global ? ((cur_left - 1 + nx) % nx) : (cur_left - 1);
              if (next_left < 0) {
                break;
              }
              int test_idx = y * nx + next_left;
              if (!isWetPoint(land_sea_map[test_idx])) {
                break;
              }
              if (partition_map[test_idx] != p1) {
                break;
              }
              if (getNeighborPartition(next_left, y, best_dir) != best_p2) {
                break;
              }
              cur_left = next_left;
              left_count++;
            }

            int right_count = 0;
            int cur_right = x;
            while (left_count + right_count < nx - 1) {
              int next_right =
                  is_global ? ((cur_right + 1) % nx) : (cur_right + 1);
              if (next_right >= nx) {
                break;
              }
              int test_idx = y * nx + next_right;
              if (!isWetPoint(land_sea_map[test_idx])) {
                break;
              }
              if (partition_map[test_idx] != p1) {
                break;
              }
              if (getNeighborPartition(next_right, y, best_dir) != best_p2) {
                break;
              }
              cur_right = next_right;
              right_count++;
            }

            int total_seg = left_count + 1 + right_count;
            int cur = cur_left;
            for (int s = 0; s < total_seg; ++s) {
              segment_indices.push_back(y * nx + cur);
              cur = is_global ? ((cur + 1) % nx) : (cur + 1);
            }
          }
        } else {
          // Vertical interface: inspect adjacent wet points along column x
          bool down_wet = (y > 0) && isWetPoint(land_sea_map[(y - 1) * nx + x]);
          bool up_wet =
              (y < ny - 1) && isWetPoint(land_sea_map[(y + 1) * nx + x]);

          if (!down_wet && !up_wet) {
            // Adjacent points along column are not wet; treat as individual
            // point
            segment_indices.push_back(idx);
          } else {
            // Collect contiguous segment along column x
            int y_start = y;
            while (y_start > 0) {
              int test_idx = (y_start - 1) * nx + x;
              if (!isWetPoint(land_sea_map[test_idx])) {
                break;
              }
              if (partition_map[test_idx] != p1) {
                break;
              }
              if (getNeighborPartition(x, y_start - 1, best_dir) != best_p2) {
                break;
              }
              y_start--;
            }

            int y_end = y;
            while (y_end < ny - 1) {
              int test_idx = (y_end + 1) * nx + x;
              if (!isWetPoint(land_sea_map[test_idx])) {
                break;
              }
              if (partition_map[test_idx] != p1) {
                break;
              }
              if (getNeighborPartition(x, y_end + 1, best_dir) != best_p2) {
                break;
              }
              y_end++;
            }

            for (int r = y_start; r <= y_end; ++r) {
              segment_indices.push_back(r * nx + x);
            }
          }
        }

        int S = static_cast<int>(segment_indices.size());
        if (S == 0) {
          continue;
        }

        // Mark points as processed for this iteration pass
        for (int pt : segment_indices) {
          processed_in_iter[pt] = true;
        }

        // Imbalance condition: Do not exchange if this increases the imbalance
        // between the number of points in the two partitions involved.
        int diff_before =
            std::abs(partition_counts[p1] - partition_counts[best_p2]);
        int diff_after = std::abs((partition_counts[p1] - S) -
                                  (partition_counts[best_p2] + S));

        if (diff_after <= diff_before) {
          for (int pt : segment_indices) {
            partition_map[pt] = best_p2;
          }
          partition_counts[p1] -= S;
          partition_counts[best_p2] += S;
          swaps_made += S;
        }
      }
    }

    // Clean single protruding points after exchange
    int cleaned = cleanSingleProtrudingPoints(
        nx, ny, land_sea_map, num_partitions, partition_map, is_global);
    if (cleaned > 0) {
      swaps_made += cleaned;
      // Resynchronize partition counts
      std::fill(partition_counts.begin(), partition_counts.end(), 0);
      for (size_t i = 0; i < partition_map.size(); ++i) {
        int pid = partition_map[i];
        if (isWetPoint(land_sea_map[i]) && pid >= 1 && pid <= num_partitions) {
          partition_counts[pid]++;
        }
      }
    }

    // --- Disconnected Piece Cleanup (every iteration) -----------------------
    int reassigned = reassignDisconnectedPieces(
        nx, ny, land_sea_map, num_partitions, partition_map, is_global);
    if (reassigned > 0) {
      swaps_made += reassigned;
      std::fill(partition_counts.begin(), partition_counts.end(), 0);
      for (size_t i = 0; i < partition_map.size(); ++i) {
        int pid = partition_map[i];
        if (isWetPoint(land_sea_map[i]) && pid >= 1 && pid <= num_partitions) {
          partition_counts[pid]++;
        }
      }
    }

    // Record intermediate map if swaps occurred
    if (swaps_made > 0 && record_hook) {
      record_hook();
    }

    // === Section: Iteration Convergence Determination =======================
    if (swaps_made == 0) {
      break;
    }
  }

  return actual_iters;
}

// --- splitIsolatedPartitions ------------------------------------------------
/**
 * @brief Splits isolated overloaded partitions by merging the smallest pair of
 * neighboring partitions and using the released partition number.
 * @details Checks if an isolated partition (having no 4-connected sea cells
 * belonging to other partitions) is overloaded (holds strictly more sea points
 * than target load). If found, the routine locates the smallest pair of
 * adjacent neighboring partitions whose merger will not produce a new
 * isolated partition, merges them to release a partition ID, and bisects the
 * isolated partition into two sub-partitions using the released partition
 * number.
 *
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Partition IDs vector to update in place.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @return True if an isolated partition was split; false otherwise.
 */
[[maybe_unused]] bool splitIsolatedPartitions(int nx, int ny,
                                              std::span<const int> land_sea_map,
                                              int num_partitions,
                                              std::vector<int> &partition_map,
                                              bool is_global = false) {
  if (num_partitions <= 1 || nx <= 0 || ny <= 0) {
    return false;
  }

  // === Section: Spatial Connectivity & Partition Inventory Setup ===========
  const int dx[] = {0, 0, -1, 1};
  const int dy[] = {-1, 1, 0, 0};

  std::vector<int> partition_counts(num_partitions + 1, 0);
  int total_sea = 0;
  for (size_t i = 0; i < partition_map.size(); ++i) {
    int p = partition_map[i];
    if (isWetPoint(land_sea_map[i]) && p >= 1 && p <= num_partitions) {
      partition_counts[p]++;
      total_sea++;
    }
  }

  if (total_sea == 0) {
    return false;
  }

  double target_per_partition = static_cast<double>(total_sea) / num_partitions;
  int target_ceil = static_cast<int>(std::ceil(target_per_partition));

  // === Section: Spatial Neighborhood Mapping ================================
  std::vector<std::set<int>> partition_neighbors(num_partitions + 1);
  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      int idx = y * nx + x;
      if (!isWetPoint(land_sea_map[idx])) {
        continue;
      }
      int p1 = partition_map[idx];
      if (p1 < 1 || p1 > num_partitions) {
        continue;
      }

      for (int k = 0; k < 4; ++k) {
        int nbr_x = x + dx[k];
        int nbr_y = y + dy[k];
        if (nbr_y < 0 || nbr_y >= ny) {
          continue;
        }
        if (is_global) {
          nbr_x = (nbr_x + nx) % nx;
        } else if (nbr_x < 0 || nbr_x >= nx) {
          continue;
        }
        int nbr_idx = nbr_y * nx + nbr_x;
        if (isWetPoint(land_sea_map[nbr_idx])) {
          int p2 = partition_map[nbr_idx];
          if (p2 >= 1 && p2 <= num_partitions && p2 != p1) {
            partition_neighbors[p1].insert(p2);
          }
        }
      }
    }
  }

  // === Section: Isolated Overloaded Partition Identification ===============
  int isolated_p = -1;
  int max_isolated_count = -1;

  for (int p = 1; p <= num_partitions; ++p) {
    if (partition_counts[p] > target_ceil && partition_neighbors[p].empty()) {
      if (partition_counts[p] > max_isolated_count) {
        max_isolated_count = partition_counts[p];
        isolated_p = p;
      }
    }
  }

  if (isolated_p == -1) {
    return false;
  }

  // === Section: Candidate Pair Selection and Validation =====================
  int best_q1 = -1;
  int best_q2 = -1;
  int min_pair_sum = -1;

  for (int q1 = 1; q1 <= num_partitions; ++q1) {
    if (q1 == isolated_p || partition_counts[q1] == 0) {
      continue;
    }
    for (int q2 : partition_neighbors[q1]) {
      if (q2 <= q1 || q2 == isolated_p || partition_counts[q2] == 0) {
        continue;
      }

      auto is_external_nbr = [q1, q2, isolated_p](int nbr) {
        return nbr != q1 && nbr != q2 && nbr != isolated_p;
      };

      bool has_external_neighbor =
          std::any_of(partition_neighbors[q1].begin(),
                      partition_neighbors[q1].end(), is_external_nbr) ||
          std::any_of(partition_neighbors[q2].begin(),
                      partition_neighbors[q2].end(), is_external_nbr);

      if (!has_external_neighbor) {
        continue;
      }

      int pair_sum = partition_counts[q1] + partition_counts[q2];
      if (min_pair_sum == -1 || pair_sum < min_pair_sum) {
        min_pair_sum = pair_sum;
        best_q1 = q1;
        best_q2 = q2;
      }
    }
  }

  if (best_q1 == -1 || best_q2 == -1) {
    return false;
  }

  // === Section: Partition Merger and ID Release =============================
  int q_keep = best_q1;
  int q_released = best_q2;

  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (isWetPoint(land_sea_map[i]) && partition_map[i] == q_released) {
      partition_map[i] = q_keep;
    }
  }

  partition_counts[q_keep] += partition_counts[q_released];
  partition_counts[q_released] = 0;

  // === Section: Isolated Partition Bisection ================================
  int x_min = nx, x_max = -1, y_min = ny, y_max = -1;
  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      int idx = y * nx + x;
      if (isWetPoint(land_sea_map[idx]) && partition_map[idx] == isolated_p) {
        x_min = std::min(x_min, x);
        x_max = std::max(x_max, x);
        y_min = std::min(y_min, y);
        y_max = std::max(y_max, y);
      }
    }
  }

  int isolated_sea = partition_counts[isolated_p];
  int target_half = isolated_sea / 2;

  bool split_x = (x_max - x_min >= y_max - y_min);

  if (split_x && x_max > x_min) {
    int current_sea = 0;
    int best_x = x_min;
    int best_diff = isolated_sea + 1;

    for (int x = x_min; x < x_max; ++x) {
      int sea_in_col = 0;
      for (int y = y_min; y <= y_max; ++y) {
        int idx = y * nx + x;
        if (isWetPoint(land_sea_map[idx]) && partition_map[idx] == isolated_p) {
          sea_in_col++;
        }
      }
      current_sea += sea_in_col;
      int diff = std::abs(current_sea - target_half);
      if (diff < best_diff) {
        best_diff = diff;
        best_x = x;
      }
    }

    for (int y = y_min; y <= y_max; ++y) {
      for (int x = best_x + 1; x <= x_max; ++x) {
        int idx = y * nx + x;
        if (isWetPoint(land_sea_map[idx]) && partition_map[idx] == isolated_p) {
          partition_map[idx] = q_released;
        }
      }
    }
  } else if (y_max > y_min) {
    int current_sea = 0;
    int best_y = y_min;
    int best_diff = isolated_sea + 1;

    for (int y = y_min; y < y_max; ++y) {
      int sea_in_row = 0;
      for (int x = x_min; x <= x_max; ++x) {
        int idx = y * nx + x;
        if (isWetPoint(land_sea_map[idx]) && partition_map[idx] == isolated_p) {
          sea_in_row++;
        }
      }
      current_sea += sea_in_row;
      int diff = std::abs(current_sea - target_half);
      if (diff < best_diff) {
        best_diff = diff;
        best_y = y;
      }
    }

    for (int y = best_y + 1; y <= y_max; ++y) {
      for (int x = x_min; x <= x_max; ++x) {
        int idx = y * nx + x;
        if (isWetPoint(land_sea_map[idx]) && partition_map[idx] == isolated_p) {
          partition_map[idx] = q_released;
        }
      }
    }
  }

  return true;
}

} // namespace

// --- partitionGlobalGrid ----------------------------------------------------

std::vector<int>
partitionGlobalGrid(int nx, int ny, std::span<const int> land_sea_map,
                    int num_partitions, int max_iterations,
                    int *actual_iterations, bool is_global,
                    std::function<void(std::span<const int>)> map_recorder) {
  if (actual_iterations) {
    *actual_iterations = 0;
  }

  if (nx <= 0 || ny <= 0 || num_partitions <= 0 ||
      land_sea_map.size() != static_cast<size_t>(nx * ny)) {
    return {};
  }

  std::vector<int> partition_map(nx * ny, 0);

  // Helper to record consecutive map if it differs from the last recorded map
  std::vector<int> last_recorded_map;
  auto record_map = [&]() {
    if (!map_recorder) {
      return;
    }
    if (last_recorded_map.empty() || last_recorded_map != partition_map) {
      last_recorded_map = partition_map;
      map_recorder(partition_map);
    }
  };

  // Initial domain decomposition: Recursive Orthogonal Bisection
  recursiveBisectDomain(0, nx - 1, 0, ny - 1, nx, land_sea_map, num_partitions,
                        1, partition_map);

  // Count total wet points and compute target domain size
  int total_wet = 0;
  for (size_t i = 0; i < partition_map.size(); ++i) {
    if (isWetPoint(land_sea_map[i]) && partition_map[i] > 0) {
      total_wet++;
    }
  }

  if (total_wet == 0) {
    record_map();
    return partition_map;
  }

  int target_size = (total_wet + num_partitions - 1) / num_partitions;

  std::vector<int> partition_counts(num_partitions + 1, 0);
  std::vector<std::set<int>> partition_neighbors(num_partitions + 1);

  // Post-bisection initialization: sort domains by size, determine neighbors,
  // output diagnostics
  resortDomainsBySize(nx, ny, land_sea_map, num_partitions, partition_map,
                      is_global, partition_counts, partition_neighbors);
  record_map();

  int min_points = partition_counts[num_partitions];
  int max_points = partition_counts[1];
  double largest_pct = (static_cast<double>(max_points) / target_size) * 100.0;
  printDomainStats(min_points, max_points, largest_pct, target_size);

  // Single integrated refinement loop
  if (num_partitions > 1 && max_iterations > 0) {
    int iters_run = 0;
    for (int iter = 0; iter < max_iterations; ++iter) {
      iters_run++;
      std::vector<int> map_at_start = partition_map;

      // Step 1: Check if smallest domain has neighbors. If not & size < 70%
      // target, split off coherent part of largest domain with neighbors.
      rebalanceIsolatedSmallestDomain(
          nx, ny, land_sea_map, num_partitions, partition_map, is_global,
          partition_counts, partition_neighbors, target_size);

      // Step 2: Check if largest domain has neighbors. If not, split in two and
      // merge smallest adjacent pair with additional neighbors.
      splitAndMergeLargestIsolatedDomain(nx, ny, land_sea_map, num_partitions,
                                         partition_map, is_global,
                                         partition_counts, partition_neighbors);

      // Step 3: Sequential full boundary exchanges between adjacent ranked
      // domains (1st-2nd, 2nd-3rd, ..., down to smallest two)
      exchangeRankedBoundaries(nx, ny, land_sea_map, num_partitions,
                               partition_map, is_global, partition_counts,
                               target_size);

      // Step 4: Deal with disconnected pieces as in the present approach
      reassignDisconnectedPieces(nx, ny, land_sea_map, num_partitions,
                                 partition_map, is_global);

      // Step 5: Deal with small grid extensions (placeholder for now)
      dealWithSmallGridExtensions(nx, ny, land_sea_map, num_partitions,
                                  partition_map, is_global);

      // Step 6: Resort the domains and re-establish neighbor information. Write
      // out test info.
      resortDomainsBySize(nx, ny, land_sea_map, num_partitions, partition_map,
                          is_global, partition_counts, partition_neighbors);

      min_points = partition_counts[num_partitions];
      max_points = partition_counts[1];
      largest_pct = (static_cast<double>(max_points) / target_size) * 100.0;
      printDomainStats(min_points, max_points, largest_pct, target_size);

      record_map();

      // Step 7: Determine if convergence is reached (< 15% larger than target,
      // or no domains changed)
      bool no_domains_changed = (partition_map == map_at_start);
      bool load_balanced = (static_cast<double>(max_points) <
                            1.15 * static_cast<double>(target_size));

      if (load_balanced || no_domains_changed) {
        break;
      }
    }

    if (actual_iterations) {
      *actual_iterations = iters_run;
    }
  }

  // Final check to record map if not already recorded
  record_map();

  return partition_map;
}

} // namespace UQ_utils
} // namespace ww4_utils
