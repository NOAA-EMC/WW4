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
 * @date Last update : 2026-11-05
 */

#include "ww4_core/solver_UQ/UQ_domains.h"

#include <algorithm>
#include <cmath>
#include <functional>
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

// --- refinePartitionsIteratively --------------------------------------------
/**
 * @brief Iteratively refines partition assignments at domain boundaries to
 * balance active sea point counts across partitions.
 * @details Examines boundary sea points between overloaded and underloaded
 * adjacent partitions and reassigns points to improve overall load balance
 * until convergence or max_iterations is reached.
 *
 * Algorithm and Convergence Determination:
 * Target Sea Point Calculation:
 *   Ideal target count per partition is defined as target = total_sea /
 * num_partitions. Overloaded partitions have counts > floor(target).
 *   Underloaded partitions have counts < ceil(target).
 *
 * Boundary Scan and Point Reassignment:
 *   For each sea grid point assigned to an overloaded partition p1, the routine
 *   inspects its 4-connected spatial neighbors (north, south, west, east).
 *   If a neighbor belongs to an underloaded adjacent partition p2, the grid
 * point is reassigned from p1 to p2. Partition sea counts are updated
 * dynamically (count[p1] decreases by 1, count[p2] increases by 1).
 *
 * Convergence Criteria:
 *   An iteration pass consists of a complete sweep across all grid cells (y,
 * x). The routine tracks swaps_made during each pass. Convergence is defined as
 *   a complete pass where swaps_made == 0, meaning no further boundary point
 *   swaps between adjacent overloaded and underloaded partitions are possible.
 *   Iterative processing terminates immediately upon convergence or when
 *   max_iterations is reached.
 *
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of target partitions.
 * @param max_iterations Maximum number of boundary swap passes.
 * @param[in,out] partition_map Partition IDs vector to refine in place.
 * @return Number of actual iteration passes executed until convergence or cap.
 */
int refinePartitionsIteratively(
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
  // Calculate ideal target sea point distribution across all partitions.
  // Overloaded partitions exceed floor(target); underloaded partitions fall
  // below ceil(target).
  double target_per_partition = static_cast<double>(total_sea) / num_partitions;

  // Define 4-connected spatial neighbor direction vectors (south, north, west,
  // east)
  const int dx[] = {0, 0, -1, 1};
  const int dy[] = {-1, 1, 0, 0};

  // === Section: Iterative Refinement Loop ===================================
  // Perform boundary refinement sweeps up to the user-specified iteration
  // limit.
  int actual_iters = 0;
  for (int iter = 0; iter < max_iterations; ++iter) {
    actual_iters++;
    int swaps_made = 0;

    // Execute full 2D spatial grid sweep across all rows (y) and columns (x)
    for (int y = 0; y < ny; ++y) {
      for (int x = 0; x < nx; ++x) {
        int idx = y * nx + x;
        if (!isWetPoint(land_sea_map[idx])) {
          continue; // Skip non-wet points
        }

        int p1 = partition_map[idx];
        if (p1 < 1 || p1 > num_partitions) {
          continue;
        }

        // --- Overloaded Partition Filter ------------------------------------
        // Only shift boundary sea points away from partition p1 if p1 holds
        // strictly more sea points than the floor of the target load.
        if (partition_counts[p1] <=
            static_cast<int>(std::floor(target_per_partition))) {
          continue;
        }

        // --- Spatial 4-Neighbor Boundary Inspection -------------------------
        // Inspect 4-connected adjacent sea cells (north, south, east, west)
        for (int k = 0; k < 4; ++k) {
          int nx_nbr = x + dx[k];
          int ny_nbr = y + dy[k];

          if (ny_nbr < 0 || ny_nbr >= ny) {
            continue; // Out of latitudinal grid bounds
          }

          if (is_global) {
            nx_nbr = (nx_nbr + nx) % nx;
          } else if (nx_nbr < 0 || nx_nbr >= nx) {
            continue; // Out of longitudinal grid bounds
          }

          int nbr_idx = ny_nbr * nx + nx_nbr;
          if (!isWetPoint(land_sea_map[nbr_idx])) {
            continue; // Non-wet neighbor
          }

          int p2 = partition_map[nbr_idx];
          if (p2 < 1 || p2 > num_partitions || p2 == p1) {
            continue; // Ignore cells in same partition or invalid IDs
          }

          // --- Dynamic Load Reassignment ------------------------------------
          // Reassign boundary point from p1 to adjacent partition p2 if p2
          // is currently underloaded (holds fewer points than ceil(target)).
          if (partition_counts[p2] <
              static_cast<int>(std::ceil(target_per_partition))) {
            partition_map[idx] = p2;
            partition_counts[p1]--;
            partition_counts[p2]++;
            swaps_made++;
            break; // Move to next grid cell after successful swap
          }
        }
      }
    }

    // Record intermediate map if swaps occurred
    if (swaps_made > 0 && record_hook) {
      record_hook();
    }

    // === Section: Iteration Convergence Determination =======================
    // Convergence condition: if a complete sweep across the grid results in
    // zero boundary sea point swaps (swaps_made == 0), the boundary assignments
    // have stabilized and optimal load balance across partitions has converged.
    if (swaps_made == 0) {
      break;
    }
  }

  return actual_iters;
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
 * Algorithm and Bisection Protocol:
 * Isolated Overloaded Partition Detection:
 *   Partition sea point counts are evaluated against the target ceiling
 *   target_ceil = ceil(total_sea / num_partitions). An isolated partition p has
 *   no 4-connected sea neighbors belonging to other partitions. If p has
 *   count[p] > target_ceil, it cannot be reduced via boundary refinement and
 *   is selected for bisection.
 *
 * Candidate Merge Pair Selection:
 *   Searches for adjacent neighboring partition pairs (q1, q2) that share a
 *   4-connected sea boundary. Candidate pairs are checked to verify that
 *   merging q1 and q2 will not result in a new isolated partition (i.e. the
 *   merged partition shares a boundary with at least one external neighbor
 *   partition other than q1, q2, and isolated_p). Among valid pairs, the pair
 *   minimizing count[q1] + count[q2] is selected.
 *
 * Partition Merger and Bisection Split:
 *   Partition q2 is merged into q1, releasing partition ID q2. The bounding
 *   box of isolated partition p is computed, and p's sea cells are bisected
 *   along its major axis into two sub-partitions using IDs p and q2.
 *
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Partition IDs vector to update in place.
 * @return True if an isolated partition was split; false otherwise.
 */
bool splitIsolatedPartitions(int nx, int ny, std::span<const int> land_sea_map,
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
  // Build neighborhood map recording adjacent partition IDs for each partition
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

      // Ensure merging q1 and q2 does not create a new isolated partition.
      // The merged partition must share a boundary with at least one external
      // neighbor partition other than q1, q2, and isolated_p.
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

  recursiveBisectDomain(0, nx - 1, 0, ny - 1, nx, land_sea_map, num_partitions,
                        1, partition_map);
  // Record initial bisection map
  record_map();

  if (num_partitions > 1) {
    if (max_iterations > 0) {
      int iters_run = refinePartitionsIteratively(
          nx, ny, land_sea_map, num_partitions, max_iterations, partition_map,
          is_global, record_map);
      if (actual_iterations) {
        *actual_iterations = iters_run;
      }
    }

    int reassigned = reassignDisconnectedPieces(
        nx, ny, land_sea_map, num_partitions, partition_map, is_global);
    if (reassigned > 0) {
      record_map();
    }

    for (int split_pass = 0; split_pass < num_partitions; ++split_pass) {
      bool split_done = splitIsolatedPartitions(
          nx, ny, land_sea_map, num_partitions, partition_map, is_global);
      if (!split_done) {
        break;
      }
      record_map();
      if (max_iterations > 0) {
        refinePartitionsIteratively(nx, ny, land_sea_map, num_partitions,
                                    max_iterations, partition_map, is_global,
                                    record_map);
      }
      int reassigned_after_split = reassignDisconnectedPieces(
          nx, ny, land_sea_map, num_partitions, partition_map, is_global);
      if (reassigned_after_split > 0) {
        record_map();
      }
    }
  }

  // Final check to record map if not already recorded
  record_map();

  return partition_map;
}

} // namespace UQ_utils
} // namespace ww4_utils
