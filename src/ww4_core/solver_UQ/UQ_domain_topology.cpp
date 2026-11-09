/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file UQ_domain_topology.cpp
 * @brief Implementation of domain topology and graph analysis tools for UQ
 * partitioning.
 * @details Implements routines for neighbor identification, disconnected piece
 * detection, boundary extraction, protrusion cleanup, and grid extension
 * smoothing.
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
 * @date Initial, 2026-11-09
 * @date Last update : 2026-11-09
 */

#include "ww4_core/solver_UQ/UQ_domain_topology.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <queue>
#include <set>
#include <span>
#include <vector>

namespace ww4_core::solver_UQ {

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
    const std::vector<int> &partition_map, bool is_global) {
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
                               bool is_global) {
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
 * @brief Cleans small grid extensions and isolated point protrusions.
 * @details If a point in a domain does not have points in the same domain next
 * to it in a row/column, and one of these adjacent points is in another domain,
 * assign the point to the other domain. If all three points in a row/column are
 * in different domains, move the point to the domain most common in its four
 * direct neighbors, or if that is not decisive, within the nine-point stencil.
 * Alternates row-column processing order per iteration.
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Partition assignments vector.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @param iter Current refinement loop iteration index.
 * @return Number of cell adjustments executed.
 */
int dealWithSmallGridExtensions(int nx, int ny,
                                std::span<const int> land_sea_map,
                                int num_partitions,
                                std::vector<int> &partition_map, bool is_global,
                                int iter) {
  if (num_partitions <= 1 || nx <= 0 || ny <= 0) {
    return 0;
  }

  int total_changes = 0;

  auto get_domain = [&](int x, int y) -> int {
    if (y < 0 || y >= ny) {
      return 0;
    }
    if (is_global) {
      x = (x % nx + nx) % nx;
    } else if (x < 0 || x >= nx) {
      return 0;
    }
    int idx = y * nx + x;
    if (!isWetPoint(land_sea_map[idx])) {
      return 0;
    }
    return partition_map[idx];
  };

  auto find_most_common_neighbor = [&](int cx, int cy) -> int {
    std::map<int, int> counts_4;
    const int dx4[] = {0, 0, -1, 1};
    const int dy4[] = {-1, 1, 0, 0};
    for (int k = 0; k < 4; ++k) {
      int d = get_domain(cx + dx4[k], cy + dy4[k]);
      if (d > 0 && d <= num_partitions) {
        counts_4[d]++;
      }
    }

    int best_d = -1;
    int max_c = 0;
    bool tie_4 = false;
    for (const auto &[d, c] : counts_4) {
      if (c > max_c) {
        max_c = c;
        best_d = d;
        tie_4 = false;
      } else if (c == max_c) {
        tie_4 = true;
      }
    }

    if (!tie_4 && best_d != -1) {
      return best_d;
    }

    std::map<int, int> counts_9;
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dx = -1; dx <= 1; ++dx) {
        int d = get_domain(cx + dx, cy + dy);
        if (d > 0 && d <= num_partitions) {
          counts_9[d]++;
        }
      }
    }

    int best_9 = -1;
    int max_9 = 0;
    for (const auto &[d, c] : counts_9) {
      if (c > max_9) {
        max_9 = c;
        best_9 = d;
      }
    }
    return (best_9 != -1) ? best_9 : best_d;
  };

  auto row_sweep = [&]() {
    int changes = 0;
    for (int y = 0; y < ny; ++y) {
      for (int x = 0; x < nx; ++x) {
        int idx = y * nx + x;
        if (!isWetPoint(land_sea_map[idx])) {
          continue;
        }
        int p = partition_map[idx];
        if (p < 1 || p > num_partitions) {
          continue;
        }

        int left_x = is_global ? ((x - 1 + nx) % nx) : (x - 1);
        int right_x = is_global ? ((x + 1) % nx) : (x + 1);

        int p_left = get_domain(left_x, y);
        int p_right = get_domain(right_x, y);

        if (p_left != p && p_right != p) {
          if (p_left > 0 && p_right > 0 && p_left != p_right) {
            int new_d = find_most_common_neighbor(x, y);
            if (new_d > 0 && new_d != p) {
              partition_map[idx] = new_d;
              changes++;
            }
          } else if (p_left > 0 && p_right > 0 && p_left == p_right) {
            partition_map[idx] = p_left;
            changes++;
          } else if (p_left > 0 && p_right == 0) {
            partition_map[idx] = p_left;
            changes++;
          } else if (p_right > 0 && p_left == 0) {
            partition_map[idx] = p_right;
            changes++;
          }
        }
      }
    }
    return changes;
  };

  auto col_sweep = [&]() {
    int changes = 0;
    for (int y = 0; y < ny; ++y) {
      for (int x = 0; x < nx; ++x) {
        int idx = y * nx + x;
        if (!isWetPoint(land_sea_map[idx])) {
          continue;
        }
        int p = partition_map[idx];
        if (p < 1 || p > num_partitions) {
          continue;
        }

        int p_bottom = get_domain(x, y - 1);
        int p_top = get_domain(x, y + 1);

        if (p_bottom != p && p_top != p) {
          if (p_bottom > 0 && p_top > 0 && p_bottom != p_top) {
            int new_d = find_most_common_neighbor(x, y);
            if (new_d > 0 && new_d != p) {
              partition_map[idx] = new_d;
              changes++;
            }
          } else if (p_bottom > 0 && p_top > 0 && p_bottom == p_top) {
            partition_map[idx] = p_bottom;
            changes++;
          } else if (p_bottom > 0 && p_top == 0) {
            partition_map[idx] = p_bottom;
            changes++;
          } else if (p_top > 0 && p_bottom == 0) {
            partition_map[idx] = p_top;
            changes++;
          }
        }
      }
    }
    return changes;
  };

  if (iter % 2 == 0) {
    total_changes += row_sweep();
    total_changes += col_sweep();
  } else {
    total_changes += col_sweep();
    total_changes += row_sweep();
  }

  return total_changes;
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
                            bool is_global) {
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

} // namespace ww4_core::solver_UQ
