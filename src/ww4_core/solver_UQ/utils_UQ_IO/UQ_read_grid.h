//+ -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
//-- | WAVEWATCH IV, open source, code management by NOAA / NWS |
//+ -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
//--
//
// @file UQ_read_grid.h
// @brief NetCDF-UGRID 1.0 grid file reader subroutine interface for WAVEWATCH
// IV.
// @details Header declaring data structures and the readUQgrid subroutine for
// reading NetCDF-UGRID grid files in WAVEWATCH IV.
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

#ifndef WW4_UQ_READ_GRID_H
#define WW4_UQ_READ_GRID_H

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace ww4::io {

/**
 * @brief Container structure holding data and metadata read from a WW4
 * NetCDF-UGRID grid file.
 */
struct UgridGrid {
  /// 2D grid y-dimension (number of rows)
  std::size_t ny{0};
  /// 2D grid x-dimension (number of columns)
  std::size_t nx{0};
  /// Number of mesh face cells (ny * nx)
  std::size_t n_faces{0};
  /// Number of mesh nodes
  std::size_t n_nodes{0};
  /// Maximum number of nodes per face cell (typically 4)
  std::size_t max_face_nodes{0};

  /// Global title attribute
  std::string title;
  /// Global source attribute
  std::string source;
  /// Global grid_type attribute
  std::string grid_type;
  /// Global coord_type attribute
  std::string coord_type;
  /// Global global_closure attribute
  std::string global_closure;
  /// Global bathy_source attribute
  std::string bathy_source;
  /// Global bound_source attribute
  std::string bound_source;
  /// Global Conventions attribute
  std::string conventions;
  /// Global history attribute
  std::string history;

  /// Complete map of all global attributes
  std::map<std::string, std::string> global_attrs;

  /// Flag indicating whether subgrid x-obstruction array is present
  bool has_obsx{false};
  /// Flag indicating whether subgrid y-obstruction array is present
  bool has_obsy{false};

  /// 2D longitude grid coordinates (degrees_east), shape (ny, nx)
  std::vector<double> grid_lon;
  /// 2D latitude grid coordinates (degrees_north), shape (ny, nx)
  std::vector<double> grid_lat;
  /// 2D bathymetry depth grid (meters), shape (ny, nx)
  std::vector<float> grid_depth;
  /// 2D land/sea mask grid (0=dry, 1=wet), shape (ny, nx)
  std::vector<int32_t> grid_mask;
  /// Optional 2D subgrid x-obstruction grid, shape (ny, nx)
  std::vector<float> grid_ObsX;
  /// Optional 2D subgrid y-obstruction grid, shape (ny, nx)
  std::vector<float> grid_ObsY;

  /// 1D UGRID face center longitudes (n_faces)
  std::vector<double> face_lon;
  /// 1D UGRID face center latitudes (n_faces)
  std::vector<double> face_lat;
  /// 1D UGRID face depth values (n_faces)
  std::vector<float> depth;
  /// 1D UGRID face mask values (n_faces)
  std::vector<int32_t> mask;
  /// Optional 1D UGRID face x-obstruction values (n_faces)
  std::vector<float> ObsX;
  /// Optional 1D UGRID face y-obstruction values (n_faces)
  std::vector<float> ObsY;

  /// 1D UGRID mesh node longitudes (n_nodes)
  std::vector<double> node_lon;
  /// 1D UGRID mesh node latitudes (n_nodes)
  std::vector<double> node_lat;

  /// Face-node connectivity map (n_faces * max_face_nodes)
  std::vector<int32_t> face_node_connectivity;

  /// Clears all grid metadata and data fields.
  void clear() noexcept {
    ny = 0;
    nx = 0;
    n_faces = 0;
    n_nodes = 0;
    max_face_nodes = 0;
    title.clear();
    source.clear();
    grid_type.clear();
    coord_type.clear();
    global_closure.clear();
    bathy_source.clear();
    bound_source.clear();
    conventions.clear();
    history.clear();
    global_attrs.clear();
    has_obsx = false;
    has_obsy = false;
    grid_lon.clear();
    grid_lat.clear();
    grid_depth.clear();
    grid_mask.clear();
    grid_ObsX.clear();
    grid_ObsY.clear();
    face_lon.clear();
    face_lat.clear();
    depth.clear();
    mask.clear();
    ObsX.clear();
    ObsY.clear();
    node_lon.clear();
    node_lat.clear();
    face_node_connectivity.clear();
  }
};

/**
 * @brief Subroutine to read a WAVEWATCH IV NetCDF-UGRID 1.0 grid file using
 * netcdf-c API.
 *
 * @details Opens the NetCDF file specified by `filepath` in read-only mode
 * using netcdf-c API, verifies file existence, parses all metadata attributes
 * and grid dimensions, and reads 2D/1D curvilinear grid coordinate and field
 * arrays and mesh topology. Returns an integer status code: 0 for success, 1
 * for file not found, 2 for failure to read metadata, and 3 for failure to read
 * grid data.
 *
 * @param[in]  filepath Path to the NetCDF-UGRID file to read.
 * @param[out] grid     UgridGrid struct populated with file metadata and data
 * arrays on success.
 * @return int Status code (0 = success, 1 = file not found, 2 = metadata
 * failure, 3 = grid data failure).
 * @pre File must exist and be a valid NetCDF-UGRID file.
 * @post On success (return code 0), `grid` fields are populated with file
 * contents. On failure, `grid` is cleared.
 */
[[nodiscard]] int readUQgrid(const std::filesystem::path &filepath,
                             UgridGrid &grid);

} // namespace ww4::io

#endif // WW4_UQ_READ_GRID_H
