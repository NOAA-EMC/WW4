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

// --- cleanSingleProtrudingPoints --------------------------------------------
/**
 * @brief Checks outer boundaries of partitions for single protruding points
 * and returns such points to the adjacent grid if possible.
 * @details A single protruding point is defined as an active wet cell belonging
 * to partition p that has at most 1 orthogonal neighbor in partition p (or is
 * isolated from its partition bulk along that edge) and is adjacent to another
 * partition. Returning such points to an adjacent partition smooths jagged
 * outer boundaries without increasing partition imbalance when feasible.
 *
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Partition IDs vector to update in place.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @return Total number of protruding points reassigned.
 */
int cleanSingleProtrudingPoints(int nx, int ny,
                                std::span<const int> land_sea_map,
                                int num_partitions,
                                std::vector<int> &partition_map,
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
          bool left_wet = (x > 0) && isWetPoint(land_sea_map[y * nx + (x - 1)]);
          bool right_wet =
              (x < nx - 1) && isWetPoint(land_sea_map[y * nx + (x + 1)]);

          if (!left_wet && !right_wet) {
            // Adjacent points along the boundary row are not wet; treat as
            // individual point
            segment_indices.push_back(idx);
          } else {
            // Collect contiguous segment of adjacent wet points in this row
            // belonging to p1 that border best_p2 in direction best_dir
            int x_start = x;
            while (x_start > 0) {
              int test_idx = y * nx + (x_start - 1);
              if (!isWetPoint(land_sea_map[test_idx])) {
                break;
              }
              if (partition_map[test_idx] != p1) {
                break;
              }
              if (getNeighborPartition(x_start - 1, y, best_dir) != best_p2) {
                break;
              }
              x_start--;
            }

            int x_end = x;
            while (x_end < nx - 1) {
              int test_idx = y * nx + (x_end + 1);
              if (!isWetPoint(land_sea_map[test_idx])) {
                break;
              }
              if (partition_map[test_idx] != p1) {
                break;
              }
              if (getNeighborPartition(x_end + 1, y, best_dir) != best_p2) {
                break;
              }
              x_end++;
            }

            for (int col = x_start; col <= x_end; ++col) {
              segment_indices.push_back(y * nx + col);
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

    // Record intermediate map if swaps occurred
    if (swaps_made > 0 && record_hook) {
      record_hook();
    }

    // --- Periodic Disconnected Piece Cleanup (every 5th iteration) ---------
    if ((iter + 1) % 5 == 0) {
      int reassigned = reassignDisconnectedPieces(
          nx, ny, land_sea_map, num_partitions, partition_map, is_global);
      if (reassigned > 0) {
        std::fill(partition_counts.begin(), partition_counts.end(), 0);
        for (size_t i = 0; i < partition_map.size(); ++i) {
          int pid = partition_map[i];
          if (isWetPoint(land_sea_map[i]) && pid >= 1 &&
              pid <= num_partitions) {
            partition_counts[pid]++;
          }
        }
        if (record_hook) {
          record_hook();
        }
      }
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

    // Clean protruding points on final partition map
    int cleaned = cleanSingleProtrudingPoints(
        nx, ny, land_sea_map, num_partitions, partition_map, is_global);
    if (cleaned > 0) {
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
      int cleaned_after_split = cleanSingleProtrudingPoints(
          nx, ny, land_sea_map, num_partitions, partition_map, is_global);
      if (cleaned_after_split > 0) {
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
