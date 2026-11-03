//+ -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
//-- | WAVEWATCH IV, open source, code management by NOAA / NWS |
//+ -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
//--
//
// @file UQ_write_partition.h
// @brief NetCDF partition map writer tools for WAVEWATCH IV UQ solver.
// @details Header declaring subroutines to open a NetCDF partition map file,
// populate metadata from the full grid and partition configuration, and
// incrementally write one or more partition maps to document map evolution.
//
// @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
// Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks
// of the National Weather Service.
// NWS often uses Generative AI (GenAI) for code development and refactoring.
// Whenever GenAI is used, NWS requires a full human review of code before it
// is added to its repositories.
// @author Main Author(s) : Aldgisl (AI Persona), Hendrik L. Tolman
// @author Contributors : Jules (Agentic AI)
// @date Initial : 2026-11-03
// @date Last update : 2026-11-03

#ifndef WW4_UQ_WRITE_PARTITION_H
#define WW4_UQ_WRITE_PARTITION_H

#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_read_grid.h"
#include <cstdint>
#include <filesystem>
#include <span>

namespace ww4::io {

// --- openPartitionMapFile ---------------------------------------------------
/**
 * @brief Creates a new NetCDF partition map file and writes header and grid
 * metadata.
 *
 * @details Creates a NetCDF file at `filepath` with an unlimited dimension
 * `n_partition_maps`, along with spatial grid dimensions `ny` and `nx`. Writes
 * all grid metadata and global attributes inherited from `grid`, sets
 * `num_partitions`, and defines the partition map variable. Leaves the file
 * open in data mode ready for writing map records.
 *
 * @param[in]  filepath       Destination file path.
 * @param[in]  grid           UgridGrid structure providing metadata and
 *                            dimensions.
 * @param[in]  num_partitions Total number of partitions created.
 * @param[out] ncid           Open NetCDF file ID returned upon success.
 * @return int Status code (0 = success, 1 = file creation failure, 2 = metadata
 * or definition failure).
 */
[[nodiscard]] int openPartitionMapFile(const std::filesystem::path &filepath,
                                       const UgridGrid &grid,
                                       int num_partitions, int &ncid);

// --- writeSinglePartitionMap ------------------------------------------------
/**
 * @brief Appends a single partition map to an open partition map NetCDF file.
 *
 * @details Writes a 2D partition map array of size `ny * nx` into the open
 * NetCDF dataset at the record position designated by `map_index`. Upon
 * successful write, increments `map_index`.
 *
 * @param[in]     ncid          Open NetCDF file ID (from
 * openPartitionMapFile).
 * @param[in]     partition_map 2D partition ID slice (size ny * nx).
 * @param[in,out] map_index     Zero-based record index to write; incremented on
 *                              success.
 * @return int Status code (0 = success, 3 = data writing failure).
 */
[[nodiscard]] int
writeSinglePartitionMap(int ncid, std::span<const int32_t> partition_map,
                        std::size_t &map_index);

// --- closePartitionMapFile --------------------------------------------------
/**
 * @brief Closes an open partition map NetCDF file.
 *
 * @param[in] ncid Open NetCDF file ID.
 * @return int Status code (0 = success, non-zero on netcdf error).
 */
[[nodiscard]] int closePartitionMapFile(int ncid);

// --- writePartitionMapNetCDF ------------------------------------------------
/**
 * @brief Convenience one-shot routine to write a partition map file with a
 * single map.
 *
 * @param[in] filepath       Destination file path.
 * @param[in] grid           UgridGrid structure providing metadata and
 *                           dimensions.
 * @param[in] num_partitions Total number of partitions created.
 * @param[in] partition_map  2D partition map slice (size ny * nx).
 * @return int Status code (0 = success, 1 = open failure, 2 = metadata failure,
 * 3 = data write failure).
 */
[[nodiscard]] int
writePartitionMapNetCDF(const std::filesystem::path &filepath,
                        const UgridGrid &grid, int num_partitions,
                        std::span<const int32_t> partition_map);

} // namespace ww4::io

#endif // WW4_UQ_WRITE_PARTITION_H
