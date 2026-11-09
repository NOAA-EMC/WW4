/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file UQ_read_partition.h
 * @brief NetCDF partition map reader tools for WAVEWATCH IV UQ solver.
 * @details Header declaring data structures and subroutines to read a NetCDF
 * partition map file, with error capturing conforming to readUQgrid
 * conventions. If multiple partition maps are present in the file, outputs the
 * last map stored in the file.
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

#ifndef WW4_UQ_READ_PARTITION_H
#define WW4_UQ_READ_PARTITION_H

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace ww4::io {

/**
 * @brief Container structure holding partition map data and metadata read from
 * a partition map NetCDF file.
 */
struct PartitionMapData {
  /// Grid y-dimension (number of rows)
  std::size_t ny{0};
  /// Grid x-dimension (number of columns)
  std::size_t nx{0};
  /// Number of mesh face cells
  std::size_t n_faces{0};
  /// Number of mesh nodes
  std::size_t n_nodes{0};
  /// Maximum number of nodes per face cell
  std::size_t max_face_nodes{0};
  /// Number of partition maps present in the file
  std::size_t num_maps{0};
  /// Number of partitions created
  int num_partitions{0};

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

  /// Partition map array of the last partition map in the file (size ny * nx)
  std::vector<int32_t> partition_map;

  /// Clears all fields.
  void clear() noexcept {
    ny = 0;
    nx = 0;
    n_faces = 0;
    n_nodes = 0;
    max_face_nodes = 0;
    num_maps = 0;
    num_partitions = 0;
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
    partition_map.clear();
  }
};

// --- readUQpartitionMapNetCDF -----------------------------------------------
/**
 * @brief Reads partition map metadata and the last partition map from a NetCDF
 * file for the UQ solver.
 *
 * @details Opens the file at `filepath` using netcdf-c API, verifies existence
 * (status 1 on missing/unopenable file), parses dimensions `ny`, `nx`, and
 * `n_partition_maps` as well as metadata attributes (status 2 on failure), and
 * reads the final 2D slice from `partition_map` (status 3 on failure).
 * If multiple partition maps are present in the file, outputs the last map
 * printed in the file.
 *
 * @param[in]  filepath  Path to the NetCDF file to read.
 * @param[out] part_data Container populated with metadata, partition count,
 *                       dimensions, and the last partition map.
 * @return int Status code (0 = success, 1 = file not found / cannot open,
 * 2 = metadata / dimension read failure, 3 = dataset array read failure).
 * @pre File must exist and contain valid NetCDF partition map data.
 * @post On success (return code 0), `part_data` contains the last map slice. On
 * failure, `part_data` is cleared.
 */
[[nodiscard]] int
readUQpartitionMapNetCDF(const std::filesystem::path &filepath,
                         PartitionMapData &part_data);

// --- readPartitionMapNetCDF -------------------------------------------------
/**
 * @brief Backward-compatible alias for readUQpartitionMapNetCDF.
 *
 * @param[in]  filepath  Path to the NetCDF file to read.
 * @param[out] part_data Container populated with metadata, partition count,
 *                       dimensions, and the last partition map.
 * @return int Status code.
 */
inline int readPartitionMapNetCDF(const std::filesystem::path &filepath,
                                  PartitionMapData &part_data) {
  return readUQpartitionMapNetCDF(filepath, part_data);
}

} // namespace ww4::io

#endif // WW4_UQ_READ_PARTITION_H
