//+ -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
//-- | WAVEWATCH IV, open source, code management by NOAA / NWS |
//+ -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
//--
//
// @file UQ_read_partition.cpp
// @brief NetCDF partition map reader implementation for WAVEWATCH IV UQ solver.
// @details Implementation of reading partition map metadata and data slices
// using netcdf-c API, with error codes mirroring readUQgrid.
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

#include "UQ_read_partition.h"

#include <netcdf.h>

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace ww4::io {

namespace {

/**
 * @brief RAII wrapper for NetCDF file handle `ncid`.
 */
class NcReadHandle {
public:
  explicit NcReadHandle(const int id) noexcept : ncid_(id) {}

  ~NcReadHandle() {
    if (ncid_ >= 0) {
      nc_close(ncid_);
    }
  }

  NcReadHandle(const NcReadHandle &) = delete;
  NcReadHandle &operator=(const NcReadHandle &) = delete;

  NcReadHandle(NcReadHandle &&other) noexcept : ncid_(other.ncid_) {
    other.ncid_ = -1;
  }

  NcReadHandle &operator=(NcReadHandle &&other) noexcept {
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
 * @brief Checks NetCDF status code.
 *
 * @param[in] status Status code returned by netcdf function.
 * @return true if status is NC_NOERR, false otherwise.
 */
bool check_nc_read_err(const int status) { return (status == NC_NOERR); }

/**
 * @brief Utility function to read all global attributes into a map.
 *
 * @param[in] ncid Open NetCDF file ID.
 * @return Map of attribute names to string values.
 */
[[nodiscard]] std::map<std::string, std::string>
read_all_partition_global_attributes(const int ncid) {
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

} // namespace

// --- readPartitionMapNetCDF -------------------------------------------------
int readPartitionMapNetCDF(const std::filesystem::path &filepath,
                           PartitionMapData &part_data) {
  part_data.clear();

  // Return code 1: File not found or failed opening
  std::error_code ec;
  if (!std::filesystem::exists(filepath, ec) || ec) {
    return 1;
  }

  int raw_ncid = -1;
  const int open_status =
      nc_open(filepath.string().c_str(), NC_NOWRITE, &raw_ncid);
  if (open_status != NC_NOERR) {
    return 1;
  }
  const NcReadHandle nc_handle(raw_ncid);
  const int ncid = nc_handle.get();

  // Return code 2: Failure to read metadata (dimensions, global attributes)
  auto read_dim_len = [ncid](const char *dimname, std::size_t &len) -> bool {
    int dimid = -1;
    const int status = nc_inq_dimid(ncid, dimname, &dimid);
    if (status != NC_NOERR) {
      return false;
    }
    return check_nc_read_err(nc_inq_dimlen(ncid, dimid, &len));
  };

  if (!read_dim_len("ny", part_data.ny) || !read_dim_len("nx", part_data.nx)) {
    part_data.clear();
    return 2;
  }

  // Look for n_partition_maps dimension (defaulting to 1 if missing but 2D)
  if (!read_dim_len("n_partition_maps", part_data.num_maps)) {
    part_data.num_maps = 1;
  }

  if (part_data.num_maps == 0 || part_data.ny == 0 || part_data.nx == 0) {
    part_data.clear();
    return 2;
  }

  // Read num_partitions attribute
  int npart = 0;
  if (!check_nc_read_err(
          nc_get_att_int(ncid, NC_GLOBAL, "num_partitions", &npart))) {
    part_data.clear();
    return 2;
  }
  part_data.num_partitions = npart;

  // Read global attributes
  part_data.global_attrs = read_all_partition_global_attributes(ncid);
  auto get_attr = [&part_data](const std::string &key) -> std::string {
    auto it = part_data.global_attrs.find(key);
    return (it != part_data.global_attrs.end()) ? it->second : "";
  };

  part_data.title = get_attr("title");
  part_data.source = get_attr("source");
  part_data.grid_type = get_attr("grid_type");
  part_data.coord_type = get_attr("coord_type");
  part_data.global_closure = get_attr("global_closure");
  part_data.bathy_source = get_attr("bathy_source");
  part_data.bound_source = get_attr("bound_source");
  part_data.conventions = get_attr("Conventions");
  part_data.history = get_attr("history");

  // Return code 3: Failure to read partition map data
  int var_map = -1;
  if (!check_nc_read_err(nc_inq_varid(ncid, "partition_map", &var_map))) {
    part_data.clear();
    return 3;
  }

  int var_ndims = 0;
  if (!check_nc_read_err(nc_inq_varndims(ncid, var_map, &var_ndims))) {
    part_data.clear();
    return 3;
  }

  const std::size_t num_elements = part_data.ny * part_data.nx;
  part_data.partition_map.resize(num_elements);

  if (var_ndims == 3) {
    // Read the last partition map printed in the file (map_index = num_maps -
    // 1)
    const std::size_t last_map_idx = part_data.num_maps - 1;
    const std::size_t start[3] = {last_map_idx, 0, 0};
    const std::size_t count[3] = {1, part_data.ny, part_data.nx};

    if (!check_nc_read_err(nc_get_vara_int(
            ncid, var_map, start, count,
            reinterpret_cast<int *>(part_data.partition_map.data())))) {
      part_data.clear();
      return 3;
    }
  } else if (var_ndims == 2) {
    if (!check_nc_read_err(nc_get_var_int(
            ncid, var_map,
            reinterpret_cast<int *>(part_data.partition_map.data())))) {
      part_data.clear();
      return 3;
    }
  } else {
    part_data.clear();
    return 3;
  }

  return 0;
}

} // namespace ww4::io
