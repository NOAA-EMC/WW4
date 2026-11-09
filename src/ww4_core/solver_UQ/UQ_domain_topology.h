/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file UQ_domain_topology.h
 * @brief Internal domain topology and graph analysis tools for UQ grid
 * partitioning.
 * @details Provides subroutines for neighbor identification, disconnected piece
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

#pragma once

#include <set>
#include <span>
#include <vector>

namespace ww4_core::solver_UQ {

// --- isWetPoint -------------------------------------------------------------
/**
 * @brief Checks if a grid mask value represents an active wet calculation
 * point.
 * @param mask_val Mask value (1 = wet calculation point, 2 = boundary point).
 * @return true if point is active wet sea point, false for land or excluded.
 */
inline bool isWetPoint(int mask_val) { return mask_val == 1 || mask_val == 2; }

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
    const std::vector<int> &partition_map, bool is_global = false);

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
                         std::vector<std::set<int>> &partition_neighbors);

// --- reassignDisconnectedPieces ---------------------------------------------
/**
 * @brief Identifies disconnected components in each partition and reassigns
 * small isolated pieces to neighboring partitions.
 * @details For each partition, identifies connected components using
 * 4-connectivity (with periodic wrapping if is_global is true). If a partition
 * has multiple pieces, all pieces other than the largest one are reevaluated:
 * any piece with neighbors is reassigned to the neighboring partition that has
 * the smallest total sea-point count.
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Current partition assignments.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @param partition_counts Current wet point counts per partition ID.
 * @return Total number of cell reassignments made across all partitions.
 */
int reassignDisconnectedPieces(int nx, int ny,
                               std::span<const int> land_sea_map,
                               int num_partitions,
                               std::vector<int> &partition_map,
                               bool is_global = false);

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
                                std::vector<int> &partition_map,
                                bool is_global = false, int iter = 0);

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
                                     bool is_global, int src, int dst);

// --- cleanSingleProtrudingPoints --------------------------------------------
/**
 * @brief Identifies and reassigns single protruding boundary points.
 * @details A cell in domain A is considered a single protruding point if it has
 * at most 1 neighbor in domain A (among its 4 cardinal neighbors) and at least
 * 2 neighbors in another domain B.
 * @param nx Number of longitudinal grid columns.
 * @param ny Number of latitudinal grid rows.
 * @param land_sea_map 2D land-sea mask span.
 * @param num_partitions Total number of partitions.
 * @param[in,out] partition_map Vector of partition assignments.
 * @param is_global Flag indicating 360-degree longitudinal grid closure.
 * @return Number of protruding cells reassigned.
 */
int cleanSingleProtrudingPoints(int nx, int ny,
                                std::span<const int> land_sea_map,
                                int num_partitions,
                                std::vector<int> &partition_map,
                                bool is_global = false);

} // namespace ww4_core::solver_UQ
