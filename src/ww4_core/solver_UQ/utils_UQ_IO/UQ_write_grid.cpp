/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file UQ_write_grid.cpp
 * @brief NetCDF-UGRID 1.0 grid file writer implementation using netcdf-c for
 * WAVEWATCH IV.
 * @details Subroutine implementation for writing NetCDF-UGRID grid files in
 * WAVEWATCH IV that conform to UGRID-1.0 and can be ingested by readUQgrid.
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

#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_write_grid.h"
#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_nc_handle.h"

#include <netcdf.h>

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace ww4::io {

namespace {

/**
 * @brief Checks NetCDF status code.
 *
 * @param[in] status Status code returned by netcdf function.
 * @return true if status is NC_NOERR, false otherwise.
 */
bool check_nc_write_err(const int status) { return (status == NC_NOERR); }

} // namespace

// --- writeUQgrid ------------------------------------------------------------
int writeUQgrid(const std::filesystem::path &filepath, const UgridGrid &grid) {
  if (grid.nx == 0 || grid.ny == 0) {
    return 1;
  }

  const std::size_t num_2d = grid.nx * grid.ny;
  if (grid.grid_lon.size() != num_2d || grid.grid_lat.size() != num_2d ||
      grid.grid_depth.size() != num_2d || grid.grid_mask.size() != num_2d) {
    return 1;
  }

  // Determine face and node dimensions
  const std::size_t n_faces = (grid.n_faces > 0) ? grid.n_faces : num_2d;
  const std::size_t n_nodes =
      (grid.n_nodes > 0) ? grid.n_nodes : (grid.nx + 1) * (grid.ny + 1);
  const std::size_t max_face_nodes =
      (grid.max_face_nodes > 0) ? grid.max_face_nodes : 4;

  // Ensure parent directory exists if specified
  if (filepath.has_parent_path()) {
    std::error_code ec;
    std::filesystem::create_directories(filepath.parent_path(), ec);
  }

  int raw_ncid = -1;
  int status = nc_create(filepath.string().c_str(), NC_CLOBBER, &raw_ncid);
  if (status != NC_NOERR) {
    return 1;
  }
  const NcHandle nc_handle(raw_ncid);
  const int ncid = nc_handle.get();

  // Define dimensions
  int dim_nx = -1;
  int dim_ny = -1;
  int dim_faces = -1;
  int dim_nodes = -1;
  int dim_max_nodes = -1;

  if (!check_nc_write_err(nc_def_dim(ncid, "nx", grid.nx, &dim_nx)) ||
      !check_nc_write_err(nc_def_dim(ncid, "ny", grid.ny, &dim_ny)) ||
      !check_nc_write_err(
          nc_def_dim(ncid, "nMesh2_face", n_faces, &dim_faces)) ||
      !check_nc_write_err(
          nc_def_dim(ncid, "nMesh2_node", n_nodes, &dim_nodes)) ||
      !check_nc_write_err(nc_def_dim(ncid, "nMaxMesh2_face_nodes",
                                     max_face_nodes, &dim_max_nodes))) {
    return 2;
  }

  // Put global attributes
  auto put_attr = [ncid](const char *name, const std::string &val) -> bool {
    if (val.empty()) {
      return true;
    }
    return check_nc_write_err(
        nc_put_att_text(ncid, NC_GLOBAL, name, val.size(), val.c_str()));
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

  // Put any additional attributes from global_attrs
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

  // Define 2D variables
  const int dims_2d[2] = {dim_ny, dim_nx};
  int var_grid_lon = -1;
  int var_grid_lat = -1;
  int var_grid_depth = -1;
  int var_grid_mask = -1;
  int var_grid_obsx = -1;
  int var_grid_obsy = -1;

  if (!check_nc_write_err(
          nc_def_var(ncid, "grid_lon", NC_DOUBLE, 2, dims_2d, &var_grid_lon)) ||
      !check_nc_write_err(
          nc_def_var(ncid, "grid_lat", NC_DOUBLE, 2, dims_2d, &var_grid_lat)) ||
      !check_nc_write_err(nc_def_var(ncid, "grid_depth", NC_FLOAT, 2, dims_2d,
                                     &var_grid_depth)) ||
      !check_nc_write_err(
          nc_def_var(ncid, "grid_mask", NC_INT, 2, dims_2d, &var_grid_mask))) {
    return 2;
  }

  const bool write_obsx = grid.has_obsx || (!grid.grid_ObsX.empty() &&
                                            grid.grid_ObsX.size() == num_2d);
  const bool write_obsy = grid.has_obsy || (!grid.grid_ObsY.empty() &&
                                            grid.grid_ObsY.size() == num_2d);

  if (write_obsx) {
    if (!check_nc_write_err(nc_def_var(ncid, "grid_ObsX", NC_FLOAT, 2, dims_2d,
                                       &var_grid_obsx))) {
      return 2;
    }
  }
  if (write_obsy) {
    if (!check_nc_write_err(nc_def_var(ncid, "grid_ObsY", NC_FLOAT, 2, dims_2d,
                                       &var_grid_obsy))) {
      return 2;
    }
  }

  // Define 1D face variables
  const int dims_face[1] = {dim_faces};
  int var_face_lon = -1;
  int var_face_lat = -1;
  int var_face_depth = -1;
  int var_face_mask = -1;
  int var_face_obsx = -1;
  int var_face_obsy = -1;

  if (!check_nc_write_err(nc_def_var(ncid, "face_lon", NC_DOUBLE, 1, dims_face,
                                     &var_face_lon)) ||
      !check_nc_write_err(nc_def_var(ncid, "face_lat", NC_DOUBLE, 1, dims_face,
                                     &var_face_lat)) ||
      !check_nc_write_err(
          nc_def_var(ncid, "depth", NC_FLOAT, 1, dims_face, &var_face_depth)) ||
      !check_nc_write_err(
          nc_def_var(ncid, "mask", NC_INT, 1, dims_face, &var_face_mask))) {
    return 2;
  }

  if (write_obsx) {
    if (!check_nc_write_err(
            nc_def_var(ncid, "ObsX", NC_FLOAT, 1, dims_face, &var_face_obsx))) {
      return 2;
    }
  }
  if (write_obsy) {
    if (!check_nc_write_err(
            nc_def_var(ncid, "ObsY", NC_FLOAT, 1, dims_face, &var_face_obsy))) {
      return 2;
    }
  }

  // Define 1D node variables
  const int dims_node[1] = {dim_nodes};
  int var_node_lon = -1;
  int var_node_lat = -1;

  if (!check_nc_write_err(nc_def_var(ncid, "node_lon", NC_DOUBLE, 1, dims_node,
                                     &var_node_lon)) ||
      !check_nc_write_err(nc_def_var(ncid, "node_lat", NC_DOUBLE, 1, dims_node,
                                     &var_node_lat))) {
    return 2;
  }

  // Define connectivity variable
  const int dims_conn[2] = {dim_faces, dim_max_nodes};
  int var_conn = -1;
  if (!check_nc_write_err(nc_def_var(ncid, "face_node_connectivity", NC_INT, 2,
                                     dims_conn, &var_conn))) {
    return 2;
  }

  // End definition mode
  if (!check_nc_write_err(nc_enddef(ncid))) {
    return 2;
  }

  // Write 2D variables
  if (!check_nc_write_err(
          nc_put_var_double(ncid, var_grid_lon, grid.grid_lon.data())) ||
      !check_nc_write_err(
          nc_put_var_double(ncid, var_grid_lat, grid.grid_lat.data())) ||
      !check_nc_write_err(
          nc_put_var_float(ncid, var_grid_depth, grid.grid_depth.data())) ||
      !check_nc_write_err(nc_put_var_int(
          ncid, var_grid_mask,
          reinterpret_cast<const int *>(grid.grid_mask.data())))) {
    return 3;
  }

  if (write_obsx) {
    std::vector<float> fallback_obsx;
    const float *data_ptr = nullptr;
    if (grid.grid_ObsX.size() == num_2d) {
      data_ptr = grid.grid_ObsX.data();
    } else {
      fallback_obsx.assign(num_2d, 0.0f);
      data_ptr = fallback_obsx.data();
    }
    if (!check_nc_write_err(nc_put_var_float(ncid, var_grid_obsx, data_ptr))) {
      return 3;
    }
  }

  if (write_obsy) {
    std::vector<float> fallback_obsy;
    const float *data_ptr = nullptr;
    if (grid.grid_ObsY.size() == num_2d) {
      data_ptr = grid.grid_ObsY.data();
    } else {
      fallback_obsy.assign(num_2d, 0.0f);
      data_ptr = fallback_obsy.data();
    }
    if (!check_nc_write_err(nc_put_var_float(ncid, var_grid_obsy, data_ptr))) {
      return 3;
    }
  }

  // Write 1D face variables
  const std::vector<double> face_lon =
      (grid.face_lon.size() == n_faces) ? grid.face_lon : grid.grid_lon;
  const std::vector<double> face_lat =
      (grid.face_lat.size() == n_faces) ? grid.face_lat : grid.grid_lat;
  const std::vector<float> face_depth =
      (grid.depth.size() == n_faces) ? grid.depth : grid.grid_depth;
  const std::vector<int32_t> face_mask =
      (grid.mask.size() == n_faces) ? grid.mask : grid.grid_mask;

  if (!check_nc_write_err(
          nc_put_var_double(ncid, var_face_lon, face_lon.data())) ||
      !check_nc_write_err(
          nc_put_var_double(ncid, var_face_lat, face_lat.data())) ||
      !check_nc_write_err(
          nc_put_var_float(ncid, var_face_depth, face_depth.data())) ||
      !check_nc_write_err(
          nc_put_var_int(ncid, var_face_mask,
                         reinterpret_cast<const int *>(face_mask.data())))) {
    return 3;
  }

  if (write_obsx) {
    std::vector<float> fallback_face_obsx;
    const float *data_ptr = nullptr;
    if (grid.ObsX.size() == n_faces) {
      data_ptr = grid.ObsX.data();
    } else if (grid.grid_ObsX.size() == num_2d) {
      data_ptr = grid.grid_ObsX.data();
    } else {
      fallback_face_obsx.assign(n_faces, 0.0f);
      data_ptr = fallback_face_obsx.data();
    }
    if (!check_nc_write_err(nc_put_var_float(ncid, var_face_obsx, data_ptr))) {
      return 3;
    }
  }

  if (write_obsy) {
    std::vector<float> fallback_face_obsy;
    const float *data_ptr = nullptr;
    if (grid.ObsY.size() == n_faces) {
      data_ptr = grid.ObsY.data();
    } else if (grid.grid_ObsY.size() == num_2d) {
      data_ptr = grid.grid_ObsY.data();
    } else {
      fallback_face_obsy.assign(n_faces, 0.0f);
      data_ptr = fallback_face_obsy.data();
    }
    if (!check_nc_write_err(nc_put_var_float(ncid, var_face_obsy, data_ptr))) {
      return 3;
    }
  }

  // Write node variables
  std::vector<double> node_lon = grid.node_lon;
  std::vector<double> node_lat = grid.node_lat;
  if (node_lon.size() != n_nodes || node_lat.size() != n_nodes) {
    node_lon.assign(n_nodes, 0.0);
    node_lat.assign(n_nodes, 0.0);
  }

  if (!check_nc_write_err(
          nc_put_var_double(ncid, var_node_lon, node_lon.data())) ||
      !check_nc_write_err(
          nc_put_var_double(ncid, var_node_lat, node_lat.data()))) {
    return 3;
  }

  // Write connectivity
  std::vector<int32_t> conn = grid.face_node_connectivity;
  const std::size_t num_conn = n_faces * max_face_nodes;
  if (conn.size() != num_conn) {
    conn.assign(num_conn, 0);
    // If standard regular/curvilinear quad grid, generate default quad
    // connectivity
    if (max_face_nodes == 4 && n_faces == num_2d &&
        n_nodes == (grid.nx + 1) * (grid.ny + 1)) {
      const std::size_t n_nodes_x = grid.nx + 1;
      for (std::size_t j = 0; j < grid.ny; ++j) {
        for (std::size_t i = 0; i < grid.nx; ++i) {
          const std::size_t face_idx = j * grid.nx + i;
          conn[face_idx * 4 + 0] = static_cast<int32_t>(j * n_nodes_x + i);
          conn[face_idx * 4 + 1] =
              static_cast<int32_t>(j * n_nodes_x + (i + 1));
          conn[face_idx * 4 + 2] =
              static_cast<int32_t>((j + 1) * n_nodes_x + (i + 1));
          conn[face_idx * 4 + 3] =
              static_cast<int32_t>((j + 1) * n_nodes_x + i);
        }
      }
    }
  }

  if (!check_nc_write_err(nc_put_var_int(
          ncid, var_conn, reinterpret_cast<const int *>(conn.data())))) {
    return 3;
  }

  return 0;
}

} // namespace ww4::io
