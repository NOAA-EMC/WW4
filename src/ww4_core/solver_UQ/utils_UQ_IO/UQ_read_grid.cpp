//+ -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
//-- | WAVEWATCH IV, open source, code management by NOAA / NWS |
//+ -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
//--
//
// @file UQ_read_grid.cpp
// @brief NetCDF-UGRID 1.0 grid file reader implementation using netcdf-c for
// WAVEWATCH IV.
// @details Subroutine implementation for reading NetCDF-UGRID grid files in
// WAVEWATCH IV. Checks for file existence (status 1), metadata parsing success
// (status 2), and complete dataset reading (status 3).
//
// @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
// Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks
// of the National Weather Service.
// NWS often uses Generative AI (GenAI) for code development and refactoring.
// Whenever GenAI is used, NWS requires a full human review of code before it
// is added to its repositories.
// @author Main Author(s) : Aldgisl (AI Persona), Hendrik L. Tolman
// @author Contributors : Jules (Agentic AI)
// @date Initial : 2026-10-27
// @date Latest Update : 2026-10-30

#include "UQ_read_grid.h"

#include <netcdf.h>

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace ww4::io {

namespace {

/**
 * @brief RAII wrapper for NetCDF file handle `ncid`.
 */
class NcHandle {
public:
  explicit NcHandle(const int id) noexcept : ncid_(id) {}

  ~NcHandle() {
    if (ncid_ >= 0) {
      nc_close(ncid_);
    }
  }

  NcHandle(const NcHandle &) = delete;
  NcHandle &operator=(const NcHandle &) = delete;

  NcHandle(NcHandle &&other) noexcept : ncid_(other.ncid_) { other.ncid_ = -1; }

  NcHandle &operator=(NcHandle &&other) noexcept {
    if (this != &other) {
      if (ncid_ >= 0) {
        nc_close(ncid_);
      }
      ncid_ = other.ncid_;
      other.ncid_ = -1;
    }
    return *this;
  }

  [[nodiscard]] int get() const noexcept { return ncid_; }

private:
  int ncid_{-1};
};

/**
 * @brief Utility function to check NetCDF status and output error without
 * throwing exception.
 *
 * @param[in] status Status code returned by netcdf function.
 * @param[in] msg    Descriptive error message prefix.
 * @return true if status is NC_NOERR, false otherwise.
 */
bool check_nc_err(const int status, const std::string &msg) {
  if (status != NC_NOERR) {
    std::cerr << "Error [readUQgrid]: " << msg << ": " << nc_strerror(status)
              << "\n";
    return false;
  }
  return true;
}

/**
 * @brief Utility function to read all global attributes into a map.
 *
 * @param[in] ncid Open NetCDF file ID.
 * @return Map of attribute names to string values.
 */
[[nodiscard]] std::map<std::string, std::string>
read_all_global_attributes(const int ncid) {
  std::map<std::string, std::string> attrs;
  int num_atts = 0;
  if (nc_inq_natts(ncid, &num_atts) != NC_NOERR) {
    return attrs;
  }

  for (int i = 0; i < num_atts; ++i) {
    char name[NC_MAX_NAME + 1];
    if (nc_inq_attname(ncid, NC_GLOBAL, i, name) != NC_NOERR) {
      continue;
    }

    nc_type xtype = NC_NAT;
    std::size_t len = 0;
    if (nc_inq_att(ncid, NC_GLOBAL, name, &xtype, &len) != NC_NOERR) {
      continue;
    }

    if (xtype == NC_CHAR) {
      std::string val(len, '\0');
      if (nc_get_att_text(ncid, NC_GLOBAL, name, val.data()) == NC_NOERR) {
        while (!val.empty() && (val.back() == '\0' || val.back() == ' ')) {
          val.pop_back();
        }
        attrs[name] = val;
      }
    }
  }
  return attrs;
}

/**
 * @brief Reads data from a NetCDF variable into a std::vector.
 *
 * @tparam T Vector element data type.
 * @param[in]  ncid     Open NetCDF file ID.
 * @param[in]  varname  Variable name to read.
 * @param[out] vec      Vector to store variable values.
 * @param[in]  required Whether missing variable triggers error.
 * @return true if variable was successfully read, false otherwise.
 */
template <typename T>
bool read_variable(const int ncid, const char *varname, std::vector<T> &vec,
                   const bool required = true) {
  int varid = -1;
  const int status = nc_inq_varid(ncid, varname, &varid);
  if (status != NC_NOERR) {
    if (required) {
      std::cerr << "Error [readUQgrid]: Missing required variable '" << varname
                << "': " << nc_strerror(status) << "\n";
    }
    return false;
  }

  nc_type xtype = NC_NAT;
  if (!check_nc_err(nc_inq_vartype(ncid, varid, &xtype),
                    "Inquiring variable type for '" + std::string(varname) +
                        "'")) {
    return false;
  }

  if constexpr (std::is_same_v<T, double>) {
    if (!check_nc_err(nc_get_var_double(ncid, varid, vec.data()),
                      "Reading double variable '" + std::string(varname) +
                          "'")) {
      return false;
    }
  } else if constexpr (std::is_same_v<T, float>) {
    if (!check_nc_err(nc_get_var_float(ncid, varid, vec.data()),
                      "Reading float variable '" + std::string(varname) +
                          "'")) {
      return false;
    }
  } else if constexpr (std::is_same_v<T, int32_t>) {
    if (xtype == NC_INT64) {
      std::vector<long long> tmp_64(vec.size());
      if (!check_nc_err(nc_get_var_longlong(ncid, varid, tmp_64.data()),
                        "Reading int64 variable '" + std::string(varname) +
                            "'")) {
        return false;
      }
      for (std::size_t i = 0; i < vec.size(); ++i) {
        vec[i] = static_cast<int32_t>(tmp_64[i]);
      }
    } else {
      if (!check_nc_err(
              nc_get_var_int(ncid, varid, reinterpret_cast<int *>(vec.data())),
              "Reading int variable '" + std::string(varname) + "'")) {
        return false;
      }
    }
  }

  return true;
}

} // namespace

int readUQgrid(const std::filesystem::path &filepath, UgridGrid &grid) {
  grid.clear();

  // Return code 1: File not found
  std::error_code ec;
  if (!std::filesystem::exists(filepath, ec) || ec) {
    std::cerr << "Error [readUQgrid]: File does not exist: "
              << filepath.string() << "\n";
    return 1;
  }

  int raw_ncid = -1;
  const int open_status =
      nc_open(filepath.string().c_str(), NC_NOWRITE, &raw_ncid);
  if (open_status != NC_NOERR) {
    std::cerr << "Error [readUQgrid]: Failed opening NetCDF file "
              << filepath.string() << ": " << nc_strerror(open_status) << "\n";
    return 1;
  }
  const NcHandle nc_handle(raw_ncid);
  const int ncid = nc_handle.get();

  // Return code 2: Failure to read metadata (dimensions and global attributes)
  auto read_dim_len = [ncid](const char *dimname, std::size_t &len) -> bool {
    int dimid = -1;
    const int status = nc_inq_dimid(ncid, dimname, &dimid);
    if (status != NC_NOERR) {
      std::cerr << "Error [readUQgrid]: Missing required dimension '" << dimname
                << "': " << nc_strerror(status) << "\n";
      return false;
    }
    return check_nc_err(nc_inq_dimlen(ncid, dimid, &len),
                        "Reading dimension length for '" +
                            std::string(dimname) + "'");
  };

  if (!read_dim_len("ny", grid.ny) || !read_dim_len("nx", grid.nx) ||
      !read_dim_len("nMesh2_face", grid.n_faces) ||
      !read_dim_len("nMesh2_node", grid.n_nodes) ||
      !read_dim_len("nMaxMesh2_face_nodes", grid.max_face_nodes)) {
    std::cerr
        << "Error [readUQgrid]: Failed reading metadata dimensions from file "
        << filepath.string() << "\n";
    grid.clear();
    return 2;
  }

  // Read all global attributes
  grid.global_attrs = read_all_global_attributes(ncid);
  auto get_attr = [&grid](const std::string &key) -> std::string {
    auto it = grid.global_attrs.find(key);
    return (it != grid.global_attrs.end()) ? it->second : "";
  };

  grid.title = get_attr("title");
  grid.source = get_attr("source");
  grid.grid_type = get_attr("grid_type");
  grid.coord_type = get_attr("coord_type");
  grid.global_closure = get_attr("global_closure");
  grid.bathy_source = get_attr("bathy_source");
  grid.bound_source = get_attr("bound_source");
  grid.conventions = get_attr("Conventions");
  grid.history = get_attr("history");

  // Return code 3: Failure to read grid data (all data arrays)
  const std::size_t num_2d = grid.ny * grid.nx;
  const std::size_t num_faces = grid.n_faces;
  const std::size_t num_nodes = grid.n_nodes;
  const std::size_t num_conn = grid.n_faces * grid.max_face_nodes;

  // Allocate 2D grid vectors
  grid.grid_lon.resize(num_2d);
  grid.grid_lat.resize(num_2d);
  grid.grid_depth.resize(num_2d);
  grid.grid_mask.resize(num_2d);

  // Read 2D grid variables
  if (!read_variable(ncid, "grid_lon", grid.grid_lon, true) ||
      !read_variable(ncid, "grid_lat", grid.grid_lat, true) ||
      !read_variable(ncid, "grid_depth", grid.grid_depth, true) ||
      !read_variable(ncid, "grid_mask", grid.grid_mask, true)) {
    std::cerr << "Error [readUQgrid]: Failed reading 2D grid arrays from file "
              << filepath.string() << "\n";
    grid.clear();
    return 3;
  }

  // Allocate 1D face vectors
  grid.face_lon.resize(num_faces);
  grid.face_lat.resize(num_faces);
  grid.depth.resize(num_faces);
  grid.mask.resize(num_faces);

  // Read 1D face variables
  if (!read_variable(ncid, "face_lon", grid.face_lon, true) ||
      !read_variable(ncid, "face_lat", grid.face_lat, true) ||
      !read_variable(ncid, "depth", grid.depth, true) ||
      !read_variable(ncid, "mask", grid.mask, true)) {
    std::cerr << "Error [readUQgrid]: Failed reading 1D face arrays from file "
              << filepath.string() << "\n";
    grid.clear();
    return 3;
  }

  // Allocate 1D node vectors
  grid.node_lon.resize(num_nodes);
  grid.node_lat.resize(num_nodes);

  // Read 1D node coordinates
  if (!read_variable(ncid, "node_lon", grid.node_lon, true) ||
      !read_variable(ncid, "node_lat", grid.node_lat, true)) {
    std::cerr
        << "Error [readUQgrid]: Failed reading mesh node coordinates from file "
        << filepath.string() << "\n";
    grid.clear();
    return 3;
  }

  // Read connectivity
  grid.face_node_connectivity.resize(num_conn);
  if (!read_variable(ncid, "face_node_connectivity",
                     grid.face_node_connectivity, true)) {
    std::cerr
        << "Error [readUQgrid]: Failed reading connectivity array from file "
        << filepath.string() << "\n";
    grid.clear();
    return 3;
  }

  // Read optional subgrid obstruction factors
  grid.grid_ObsX.resize(num_2d);
  grid.ObsX.resize(num_faces);
  if (read_variable(ncid, "grid_ObsX", grid.grid_ObsX, false) &&
      read_variable(ncid, "ObsX", grid.ObsX, false)) {
    grid.has_obsx = true;
  } else {
    grid.grid_ObsX.clear();
    grid.ObsX.clear();
    grid.has_obsx = false;
  }

  grid.grid_ObsY.resize(num_2d);
  grid.ObsY.resize(num_faces);
  if (read_variable(ncid, "grid_ObsY", grid.grid_ObsY, false) &&
      read_variable(ncid, "ObsY", grid.ObsY, false)) {
    grid.has_obsy = true;
  } else {
    grid.grid_ObsY.clear();
    grid.ObsY.clear();
    grid.has_obsy = false;
  }

  // Return code 0: Success
  return 0;
}

} // namespace ww4::io
