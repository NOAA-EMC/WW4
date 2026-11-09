/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file UQ_write_grid.h
 * @brief NetCDF-UGRID 1.0 grid file writer subroutine interface for WAVEWATCH
 * IV.
 * @details Header declaring subroutine for writing NetCDF-UGRID grid files in
 * WAVEWATCH IV that can subsequently be read by readUQgrid.
 *
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-11-03
 * @date Last update : 2026-11-09
 */

#ifndef WW4_UQ_WRITE_GRID_H
#define WW4_UQ_WRITE_GRID_H

#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_read_grid.h"
#include <filesystem>

namespace ww4::io {

// --- writeUQgrid ------------------------------------------------------------
/**
 * @brief Subroutine to write a WAVEWATCH IV NetCDF-UGRID 1.0 grid file using
 * netcdf-c API.
 *
 * @details Creates a NetCDF file at the specified `filepath` conforming to
 * UGRID-1.0 specifications and WAVEWATCH IV conventions. Writes all dimensions,
 * global attributes, 2D coordinates and fields (grid_lon, grid_lat, grid_depth,
 * grid_mask, and optional subgrid obstructions), 1D face coordinates and
 * fields, 1D node coordinates, and face-node connectivity. If face, node, or
 * connectivity fields in `grid` are empty, synthesizes standard quad topology
 * derived from the 2D grid coordinates.
 *
 * @param[in] filepath Path where the NetCDF file will be created.
 * @param[in] grid     UgridGrid struct containing metadata and grid data.
 * @return int Status code (0 = success, 1 = file creation failure, 2 = metadata
 * or definition failure, 3 = data writing failure).
 * @pre `grid.nx` and `grid.ny` must be greater than zero, and 2D arrays must
 * match `nx * ny`.
 * @post A valid NetCDF-UGRID file readable by `readUQgrid` is created.
 */
[[nodiscard]] int writeUQgrid(const std::filesystem::path &filepath,
                              const UgridGrid &grid);

} // namespace ww4::io

#endif // WW4_UQ_WRITE_GRID_H
