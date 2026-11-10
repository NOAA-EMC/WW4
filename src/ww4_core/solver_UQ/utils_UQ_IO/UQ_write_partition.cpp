/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file UQ_write_partition.cpp
 * @brief NetCDF partition map writer implementation for WAVEWATCH IV UQ solver.
 * @details Subroutines to create and populate NetCDF partition map files with
 * full grid metadata and one or more partition maps.
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

#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_write_partition.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_nc_handle.h"

#include <netcdf.h>

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>

namespace ww4::io {

namespace {

/**
 * @brief Checks NetCDF return status.
 *
 * @param[in] status NetCDF status code.
 * @return true if NC_NOERR, false otherwise.
 */
bool check_nc_part_err(const int status) { return (status == NC_NOERR); }

} // namespace

// --- openUQpartitionMapFile -------------------------------------------------
int openUQpartitionMapFile(const std::filesystem::path &filepath,
                           const UgridGrid &grid, int num_partitions,
                           int &ncid) {
  ncid = -1;
  if (grid.nx == 0 || grid.ny == 0 || num_partitions <= 0) {
    return 2;
  }

  if (filepath.has_parent_path()) {
    std::error_code ec;
    std::filesystem::create_directories(filepath.parent_path(), ec);
  }

  int raw_ncid = -1;
  int status = nc_create(filepath.string().c_str(), NC_CLOBBER, &raw_ncid);
  if (status != NC_NOERR) {
    return 1;
  }

  NcHandle handle(raw_ncid);

  // Determine face and node dimensions
  const std::size_t num_2d = grid.nx * grid.ny;
  const std::size_t n_faces = (grid.n_faces > 0) ? grid.n_faces : num_2d;
  const std::size_t n_nodes =
      (grid.n_nodes > 0) ? grid.n_nodes : (grid.nx + 1) * (grid.ny + 1);
  const std::size_t max_face_nodes =
      (grid.max_face_nodes > 0) ? grid.max_face_nodes : 4;

  // Define dimensions: unlimited maps dimension, plus ny, nx, and UGRID mesh
  // dimensions
  int dim_maps = -1;
  int dim_ny = -1;
  int dim_nx = -1;
  int dim_faces = -1;
  int dim_nodes = -1;
  int dim_max_nodes = -1;

  if (!check_nc_part_err(
          nc_def_dim(raw_ncid, "n_partition_maps", NC_UNLIMITED, &dim_maps)) ||
      !check_nc_part_err(nc_def_dim(raw_ncid, "ny", grid.ny, &dim_ny)) ||
      !check_nc_part_err(nc_def_dim(raw_ncid, "nx", grid.nx, &dim_nx)) ||
      !check_nc_part_err(
          nc_def_dim(raw_ncid, "nMesh2_face", n_faces, &dim_faces)) ||
      !check_nc_part_err(
          nc_def_dim(raw_ncid, "nMesh2_node", n_nodes, &dim_nodes)) ||
      !check_nc_part_err(nc_def_dim(raw_ncid, "nMaxMesh2_face_nodes",
                                    max_face_nodes, &dim_max_nodes))) {
    return 2;
  }

  // Write full grid metadata attributes
  auto put_attr = [raw_ncid](const char *name, const std::string &val) -> bool {
    if (val.empty()) {
      return true;
    }
    return check_nc_part_err(
        nc_put_att_text(raw_ncid, NC_GLOBAL, name, val.size(), val.c_str()));
  };

  if (!put_attr("title", grid.title) || !put_attr("source", grid.source) ||
      !put_attr("grid_type", grid.grid_type) ||
      !put_attr("coord_type", grid.coord_type) ||
      !put_attr("global_closure", grid.global_closure) ||
      !put_attr("bathy_source", grid.bathy_source) ||
      !put_attr("bound_source", grid.bound_source) ||
      !put_attr("Conventions", grid.conventions) ||
      !put_attr("history", grid.history)) {
    return 2;
  }

  // Put additional global attributes
  for (const auto &[key, val] : grid.global_attrs) {
    if (key == "title" || key == "source" || key == "grid_type" ||
        key == "coord_type" || key == "global_closure" ||
        key == "bathy_source" || key == "bound_source" ||
        key == "Conventions" || key == "history") {
      continue;
    }
    if (!put_attr(key.c_str(), val)) {
      return 2;
    }
  }

  // Write number of partitions as an attribute
  if (!check_nc_part_err(nc_put_att_int(raw_ncid, NC_GLOBAL, "num_partitions",
                                        NC_INT, 1, &num_partitions))) {
    return 2;
  }

  // Define 3D partition_map variable (maps, ny, nx)
  const int dims_map[3] = {dim_maps, dim_ny, dim_nx};
  int var_map = -1;
  if (!check_nc_part_err(nc_def_var(raw_ncid, "partition_map", NC_INT, 3,
                                    dims_map, &var_map))) {
    return 2;
  }

  const char *long_name = "Domain partitioning map";
  if (!check_nc_part_err(nc_put_att_text(
          raw_ncid, var_map, "long_name",
          std::char_traits<char>::length(long_name), long_name))) {
    return 2;
  }

  const int fill_val = 0;
  if (!check_nc_part_err(nc_put_att_int(raw_ncid, var_map, "_FillValue", NC_INT,
                                        1, &fill_val))) {
    return 2;
  }

  // End definition mode
  if (!check_nc_part_err(nc_enddef(raw_ncid))) {
    return 2;
  }

  ncid = handle.release();
  return 0;
}

// --- writeSingleUQpartitionMap ----------------------------------------------
int writeSingleUQpartitionMap(int ncid, std::span<const int32_t> partition_map,
                              std::size_t &map_index) {
  if (ncid < 0) {
    return 3;
  }

  int var_map = -1;
  if (!check_nc_part_err(nc_inq_varid(ncid, "partition_map", &var_map))) {
    return 3;
  }

  int dim_ny = -1;
  int dim_nx = -1;
  std::size_t ny = 0;
  std::size_t nx = 0;

  if (!check_nc_part_err(nc_inq_dimid(ncid, "ny", &dim_ny)) ||
      !check_nc_part_err(nc_inq_dimlen(ncid, dim_ny, &ny)) ||
      !check_nc_part_err(nc_inq_dimid(ncid, "nx", &dim_nx)) ||
      !check_nc_part_err(nc_inq_dimlen(ncid, dim_nx, &nx))) {
    return 3;
  }

  if (partition_map.size() != (ny * nx)) {
    return 3;
  }

  const std::size_t start[3] = {map_index, 0, 0};
  const std::size_t count[3] = {1, ny, nx};

  if (!check_nc_part_err(nc_put_vara_int(
          ncid, var_map, start, count,
          reinterpret_cast<const int *>(partition_map.data())))) {
    return 3;
  }

  ++map_index;
  return 0;
}

// --- closeUQpartitionMapFile ------------------------------------------------
int closeUQpartitionMapFile(int ncid) {
  if (ncid < 0) {
    return 0;
  }
  return nc_close(ncid);
}

// --- writeUQpartitionMapNetCDF ----------------------------------------------
int writeUQpartitionMapNetCDF(const std::filesystem::path &filepath,
                              const UgridGrid &grid, int num_partitions,
                              std::span<const int32_t> partition_map) {
  int ncid = -1;
  int status = openUQpartitionMapFile(filepath, grid, num_partitions, ncid);
  if (status != 0) {
    return status;
  }

  std::size_t map_idx = 0;
  status = writeSingleUQpartitionMap(ncid, partition_map, map_idx);
  const int close_status = closeUQpartitionMapFile(ncid);
  if (status == 0 && close_status != 0) {
    return 3;
  }
  return status;
}

} // namespace ww4::io
