/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file UQ_domains.h
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
 * @date Last update : 2026-11-09
 */

#pragma once

#include <functional>
#include <span>
#include <vector>

namespace ww4_core::solver_UQ {

// --- DomainPartition --------------------------------------------------------
/**
 * @struct DomainPartition
 * @brief Structure representing a partitioned grid domain dataset.
 */
struct DomainPartition {
  int nx{0};
  int ny{0};
  int num_partitions{0};
  bool is_global{false};
  std::vector<int> partition_map;
  std::vector<int> land_sea_map;
};

// --- partitionGlobalGrid ----------------------------------------------------
/**
 * @brief Partitions a full UQ rectangular grid into sub-domains based on
 * active sea points.
 * @details Implements recursive bisection followed by iterative boundary
 * refinement (derived from WW3 ww3_gspl) as a pure subroutine where all inputs
 * and outputs pass strictly through the function interface. Distributes sea
 * points evenly across target partitions while forming spatially contiguous
 * sub-domain blocks.
 *
 * Domain Partitioning Algorithm Lifecycle:
 * Initial Stage:
 *   Recursive bisection splits the global grid bounding box into rectangular
 *   sub-domains based on active sea point counts (where cell is wet and not
 * excluded).
 *
 * Refinement Stage (Iterative Boundary Balancing):
 *   After initial bisection, an iterative refinement phase adjusts partition
 *   boundaries to balance the sea point count across partitions. Exchanges
 *   between partitions are performed per row or column of adjacent wet points
 *   on the interface (unless adjacent boundary points are not wet) and only if
 *   the exchange does not increase the imbalance between the two partitions.
 *
 * Protruding Point Cleanup:
 *   Outer boundaries of partitions are checked for single protruding points,
 *   returning such points to adjacent partitions where possible.
 *
 * Disconnected Piece Refinement Stage and Squareness Techniques (ww4_gspl):
 *   To keep partitions as square as possible and prevent jagged boundaries,
 *   bisection evaluates domain aspect ratio along major axes, boundary swaps
 *   prioritize underloaded partitions sharing the highest neighbor
 *   connectivity (minimizing perimeter increases and preventing 1-cell
 *   peninsulas), and disconnected piece checks run periodically every 5
 *   iterations during iterative refinement.
 *   Identifies connected components for each partition using 4-connectivity.
 *   If a small piece of a partition is disconnected from the bulk of the
 *   partition, and this small piece has neighboring partitions, the small
 *   piece is reassigned to the smallest of its neighbors.
 *
 * Isolated Partition Split Stage:
 *   If a partition is too big and has no neighbors, the smallest two
 * neighboring partitions are merged, releasing a partition number that is then
 * used to split the isolated partition into two sub-partitions.
 *
 * Convergence Criteria:
 *   Iterative refinement runs for up to max_iterations passes. An iteration
 *   sweep evaluates boundaries separating an overloaded partition from an
 * adjacent underloaded partition. Convergence is achieved when a complete grid
 *   sweep results in zero point reassignments (swaps_made == 0), indicating
 *   optimal load balance across all adjacent partition boundaries.
 *
 * @param nx Number of longitudinal grid columns (x-dimension).
 * @param ny Number of latitudinal grid rows (y-dimension).
 * @param land_sea_map 2D grid status mask (row-major, size nx * ny), where
 * values 1 and 2 indicate wet sea points, 0 indicates land, and 3 indicates
 * excluded points.
 * @param num_partitions Total number of target partitions (must be >= 1).
 * @param max_iterations Maximum number of iterative refinement passes (default:
 * 200). Set to 0 to bypass iterative refinement.
 * @param[out] actual_iterations Optional pointer to store actual number of
 * refinement passes completed (returns 0 if refinement was bypassed or
 * unnecessary).
 * @param is_global Flag indicating whether grid has longitudinal closure
 * (360-degree wrapping across x = 0 and x = nx - 1) (default: false).
 * @param map_recorder Optional callback invoked with partition_map snapshots.
 * @param convergence_criterion Factor multiplied with target size to determine
 * load balance convergence (default: 1.05).
 * @return Vector of size nx * ny containing partition IDs (1 to num_partitions
 * for sea points, 0 for land).
 */
std::vector<int> partitionGlobalGrid(
    int nx, int ny, std::span<const int> land_sea_map, int num_partitions,
    int max_iterations = 200, int *actual_iterations = nullptr,
    bool is_global = false,
    std::function<void(std::span<const int>)> map_recorder = nullptr,
    double convergence_criterion = 1.05);

} // namespace ww4_core::solver_UQ

namespace ww4_utils::UQ_utils {
// Backward-compatibility namespace alias
using ww4_core::solver_UQ::DomainPartition;
using ww4_core::solver_UQ::partitionGlobalGrid;
} // namespace ww4_utils::UQ_utils
