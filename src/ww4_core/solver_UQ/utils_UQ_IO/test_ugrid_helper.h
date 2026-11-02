/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file test_ugrid_helper.h
 * @brief Helper function to create valid minimal NetCDF-UGRID files for
 * testing.
 * @copyright © 2026 National Weather Service, NOAA.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-30
 * @date Last update : 2026-10-30
 */

#pragma once

#include <cstdint>
#include <netcdf.h>
#include <string>
#include <vector>

namespace ww4::test_helpers {

inline void createMinimalUgridFile(const std::string &filename,
                                   std::size_t nx = 10, std::size_t ny = 10) {
  int ncid = -1;
  if (nc_create(filename.c_str(), NC_CLOBBER, &ncid) != NC_NOERR) {
    return;
  }
  std::size_t n_faces = nx * ny;
  std::size_t n_nodes = (nx + 1) * (ny + 1);
  std::size_t max_face_nodes = 4;
  int dim_nx = -1, dim_ny = -1, dim_faces = -1, dim_nodes = -1,
      dim_max_nodes = -1;
  nc_def_dim(ncid, "nx", nx, &dim_nx);
  nc_def_dim(ncid, "ny", ny, &dim_ny);
  nc_def_dim(ncid, "nMesh2_face", n_faces, &dim_faces);
  nc_def_dim(ncid, "nMesh2_node", n_nodes, &dim_nodes);
  nc_def_dim(ncid, "nMaxMesh2_face_nodes", max_face_nodes, &dim_max_nodes);

  const std::string title = filename;
  nc_put_att_text(ncid, NC_GLOBAL, "title", title.size(), title.c_str());

  int dims_2d[2] = {dim_ny, dim_nx};
  int var_lon = -1, var_lat = -1, var_depth = -1, var_mask = -1;
  int var_obsx = -1, var_obsy = -1;
  nc_def_var(ncid, "grid_lon", NC_DOUBLE, 2, dims_2d, &var_lon);
  nc_def_var(ncid, "grid_lat", NC_DOUBLE, 2, dims_2d, &var_lat);
  nc_def_var(ncid, "grid_depth", NC_FLOAT, 2, dims_2d, &var_depth);
  nc_def_var(ncid, "grid_mask", NC_INT, 2, dims_2d, &var_mask);
  nc_def_var(ncid, "grid_ObsX", NC_FLOAT, 2, dims_2d, &var_obsx);
  nc_def_var(ncid, "grid_ObsY", NC_FLOAT, 2, dims_2d, &var_obsy);

  int dims_face[1] = {dim_faces};
  int var_face_lon = -1, var_face_lat = -1, var_face_depth = -1,
      var_face_mask = -1;
  int var_face_obsx = -1, var_face_obsy = -1;
  nc_def_var(ncid, "face_lon", NC_DOUBLE, 1, dims_face, &var_face_lon);
  nc_def_var(ncid, "face_lat", NC_DOUBLE, 1, dims_face, &var_face_lat);
  nc_def_var(ncid, "depth", NC_FLOAT, 1, dims_face, &var_face_depth);
  nc_def_var(ncid, "mask", NC_INT, 1, dims_face, &var_face_mask);
  nc_def_var(ncid, "ObsX", NC_FLOAT, 1, dims_face, &var_face_obsx);
  nc_def_var(ncid, "ObsY", NC_FLOAT, 1, dims_face, &var_face_obsy);

  int dims_node[1] = {dim_nodes};
  int var_node_lon = -1, var_node_lat = -1;
  nc_def_var(ncid, "node_lon", NC_DOUBLE, 1, dims_node, &var_node_lon);
  nc_def_var(ncid, "node_lat", NC_DOUBLE, 1, dims_node, &var_node_lat);

  int dims_conn[2] = {dim_faces, dim_max_nodes};
  int var_conn = -1;
  nc_def_var(ncid, "face_node_connectivity", NC_INT, 2, dims_conn, &var_conn);

  nc_enddef(ncid);

  std::vector<double> lons(nx * ny);
  std::vector<double> lats(nx * ny);
  std::vector<float> depths(nx * ny, 100.0f);
  std::vector<int32_t> masks(nx * ny, 1);
  std::vector<float> obs(nx * ny, 0.0f);
  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      lons[j * nx + i] = static_cast<double>(i);
      lats[j * nx + i] = static_cast<double>(j);
    }
  }
  nc_put_var_double(ncid, var_lon, lons.data());
  nc_put_var_double(ncid, var_lat, lats.data());
  nc_put_var_float(ncid, var_depth, depths.data());
  nc_put_var_int(ncid, var_mask, masks.data());
  nc_put_var_float(ncid, var_obsx, obs.data());
  nc_put_var_float(ncid, var_obsy, obs.data());

  nc_put_var_double(ncid, var_face_lon, lons.data());
  nc_put_var_double(ncid, var_face_lat, lats.data());
  nc_put_var_float(ncid, var_face_depth, depths.data());
  nc_put_var_int(ncid, var_face_mask, masks.data());
  nc_put_var_float(ncid, var_face_obsx, obs.data());
  nc_put_var_float(ncid, var_face_obsy, obs.data());

  std::vector<double> nlons(n_nodes, 0.0);
  std::vector<double> nlats(n_nodes, 0.0);
  nc_put_var_double(ncid, var_node_lon, nlons.data());
  nc_put_var_double(ncid, var_node_lat, nlats.data());

  std::vector<int32_t> conn(n_faces * max_face_nodes, 0);
  nc_put_var_int(ncid, var_conn, conn.data());
  nc_close(ncid);
}

} // namespace ww4::test_helpers
