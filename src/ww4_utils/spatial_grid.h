/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file spatial_grid.h
 * @brief Data structures for NetCDF-UGRID spatial grid representation in
 * WAVEWATCH IV.
 * @details Defines shared grid data structures common to all grid-solvers,
 *          along with solver-specific grid data structures for Rectangular,
 *          Triangular (UGRID), and Spherical Multiple-Cell (SMC) grids.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the
 * National Weather Service. NWS often uses Generative AI (GenAI) for code
 * development and refactoring. Whenever GenAI is used, NWS requires a full
 * human review of code before it is added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-21
 * @date Last update : 2026-10-21
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace ww4_utils {

// --- GridType ---------------------------------------------------------------
/**
 * @enum GridType
 * @brief Spatial grid topology classifications in WAVEWATCH IV.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-21
 * @date Last update : 2026-10-21
 * @var GridType::Undefined
 * @brief Grid type not yet set or defined.
 * @var GridType::Rectangular
 * @brief Structured 2D rectilinear grid.
 * @var GridType::Triangular
 * @brief Unstructured triangular mesh conforming to UGRID conventions.
 * @var GridType::SMC
 * @brief Spherical Multiple-Cell (SMC) grid.
 */
enum class GridType { Undefined, Rectangular, Triangular, SMC };

// --- SharedGridData ---------------------------------------------------------
/**
 * @struct SharedGridData
 * @brief Grid data structures and physical fields shared by all grid-solvers.
 * @details Contains common spatial metadata, coordinate vectors, topobathy
 *          elevation, land/sea mask, sub-grid obstruction fields, and status
 * flags shared across all numerical wave solvers (Rectangular, Triangular,
 * SMC).
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-21
 * @date Last update : 2026-10-21
 */
struct SharedGridData {
  GridType gridType = GridType::Undefined;
  std::string name;
  std::string spatialReferenceSystem;
  bool isSpherical = true;

  std::size_t numNodes = 0;
  std::size_t numElements = 0;

  std::vector<double> xCoordinates;
  std::vector<double> yCoordinates;
  std::vector<double> topobathy;
  std::vector<int> landMask;
  std::vector<double> obstructionX;
  std::vector<double> obstructionY;
  std::vector<int> mapsta;

  /**
   * @brief Access non-owning span of longitude/x coordinates.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getXCoordinates() const noexcept {
    return xCoordinates;
  }

  /**
   * @brief Access non-owning span of latitude/y coordinates.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getYCoordinates() const noexcept {
    return yCoordinates;
  }

  /**
   * @brief Access non-owning span of the unified topobathy elevation field.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getTopobathy() const noexcept {
    return topobathy;
  }

  /**
   * @brief Access non-owning span of the land/sea mask array.
   * @return std::span<const int>
   */
  [[nodiscard]] std::span<const int> getLandMask() const noexcept {
    return landMask;
  }

  /**
   * @brief Access non-owning span of sub-grid X obstruction transmission
   * factors.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getObstructionX() const noexcept {
    return obstructionX;
  }

  /**
   * @brief Access non-owning span of sub-grid Y obstruction transmission
   * factors.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getObstructionY() const noexcept {
    return obstructionY;
  }

  /**
   * @brief Access non-owning span of status / boundary map flags.
   * @return std::span<const int>
   */
  [[nodiscard]] std::span<const int> getMapsta() const noexcept {
    return mapsta;
  }

  /**
   * @brief Resets all shared grid data arrays and fields to default state.
   */
  void clear() noexcept;
};

// --- RectangularGridData ----------------------------------------------------
/**
 * @struct RectangularGridData
 * @brief Grid data structures specific to 2D structured rectilinear solvers.
 * @details Encapsulates 2D grid dimensions (nx, ny), origin coordinates,
 *          spatial step increments, and optional non-uniform spacing vectors.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-21
 * @date Last update : 2026-10-21
 */
struct RectangularGridData {
  std::size_t nx = 0;
  std::size_t ny = 0;
  double x0 = 0.0;
  double y0 = 0.0;
  double dx = 0.0;
  double dy = 0.0;

  std::vector<double> xGridSpacing;
  std::vector<double> yGridSpacing;
  bool isGlobal = false;

  /**
   * @brief Access non-owning span of X grid spacing values.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getXGridSpacing() const noexcept {
    return xGridSpacing;
  }

  /**
   * @brief Access non-owning span of Y grid spacing values.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getYGridSpacing() const noexcept {
    return yGridSpacing;
  }

  /**
   * @brief Resets all rectangular grid specific fields to default state.
   */
  void clear() noexcept;
};

// --- TriangularGridData -----------------------------------------------------
/**
 * @struct TriangularGridData
 * @brief Grid data structures specific to NetCDF-UGRID triangular mesh solvers.
 * @details Conforms to UGRID conventions (cf_role = "mesh_topology"),
 *          storing element face-node connectivity, edge connectivity,
 * neighbor relations, element areas, and edge lengths.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-21
 * @date Last update : 2026-10-21
 */
struct TriangularGridData {
  std::string meshTopologyName;
  std::string nodeCoordinatesNames;
  std::string faceNodeConnectivityName;

  std::size_t numEdges = 0;

  std::vector<int> faceNodeConnectivity;
  std::vector<int> edgeNodeConnectivity;
  std::vector<int> faceFaceConnectivity;
  std::vector<int> faceEdgeConnectivity;

  std::vector<double> elementAreas;
  std::vector<double> edgeLengths;

  /**
   * @brief Access non-owning span of face-node connectivity.
   * @return std::span<const int>
   */
  [[nodiscard]] std::span<const int> getFaceNodeConnectivity() const noexcept {
    return faceNodeConnectivity;
  }

  /**
   * @brief Access non-owning span of edge-node connectivity.
   * @return std::span<const int>
   */
  [[nodiscard]] std::span<const int> getEdgeNodeConnectivity() const noexcept {
    return edgeNodeConnectivity;
  }

  /**
   * @brief Access non-owning span of face-face neighbor connectivity.
   * @return std::span<const int>
   */
  [[nodiscard]] std::span<const int> getFaceFaceConnectivity() const noexcept {
    return faceFaceConnectivity;
  }

  /**
   * @brief Access non-owning span of face-edge connectivity.
   * @return std::span<const int>
   */
  [[nodiscard]] std::span<const int> getFaceEdgeConnectivity() const noexcept {
    return faceEdgeConnectivity;
  }

  /**
   * @brief Access non-owning span of element surface areas.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getElementAreas() const noexcept {
    return elementAreas;
  }

  /**
   * @brief Access non-owning span of edge lengths.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getEdgeLengths() const noexcept {
    return edgeLengths;
  }

  /**
   * @brief Resets all triangular mesh specific fields to default state.
   */
  void clear() noexcept;
};

// --- SMCCell ----------------------------------------------------------------
/**
 * @struct SMCCell
 * @brief Descriptor for an active cell in a Spherical Multiple-Cell (SMC) grid.
 * @details Stores cell position on base grid, refinement level, spatial
 * extents, and area factor.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-21
 * @date Last update : 2026-10-21
 */
struct SMCCell {
  std::size_t cellId = 0;
  int iIndex = 0;
  int jIndex = 0;
  int level = 0;
  double xSize = 0.0;
  double ySize = 0.0;
  double areaFactor = 1.0;
};

// --- SMCGridData ------------------------------------------------------------
/**
 * @struct SMCGridData
 * @brief Grid data structures specific to Spherical Multiple-Cell (SMC)
 * solvers.
 * @details Encapsulates active SMC cell descriptors and directional flux
 *          transition linkages (i-face and j-face mapping tables).
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-21
 * @date Last update : 2026-10-21
 */
struct SMCGridData {
  std::size_t baseNx = 0;
  std::size_t baseNy = 0;
  std::size_t numActiveCells = 0;
  std::size_t numIFaces = 0;
  std::size_t numJFaces = 0;

  std::vector<SMCCell> cells;
  std::vector<int> iFaceLinkage;
  std::vector<int> jFaceLinkage;

  /**
   * @brief Access non-owning span of SMC cell descriptors.
   * @return std::span<const SMCCell>
   */
  [[nodiscard]] std::span<const SMCCell> getCells() const noexcept {
    return cells;
  }

  /**
   * @brief Access non-owning span of i-face flux transition linkage array.
   * @return std::span<const int>
   */
  [[nodiscard]] std::span<const int> getIFaceLinkage() const noexcept {
    return iFaceLinkage;
  }

  /**
   * @brief Access non-owning span of j-face flux transition linkage array.
   * @return std::span<const int>
   */
  [[nodiscard]] std::span<const int> getJFaceLinkage() const noexcept {
    return jFaceLinkage;
  }

  /**
   * @brief Resets all SMC grid specific fields to default state.
   */
  void clear() noexcept;
};

// --- SpecificGridData -------------------------------------------------------
/**
 * @typedef SpecificGridData
 * @brief Type-safe variant container holding solver-specific grid data.
 */
using SpecificGridData = std::variant<std::monostate, RectangularGridData,
                                      TriangularGridData, SMCGridData>;

// --- FullGrid ---------------------------------------------------------------
/**
 * @struct FullGrid
 * @brief Full model domain spatial grid metadata and data buffers.
 * @details Holds shared grid data and solver-specific grid data structures
 *          for the complete spatial domain.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-21
 * @date Last update : 2026-10-21
 */
struct FullGrid {
  SharedGridData shared;
  SpecificGridData specific;

  // --- Convenience member aliases for backward compatibility ---
  GridType gridType = GridType::Undefined;
  std::string name;
  std::size_t nx = 0;
  std::size_t ny = 0;
  std::size_t numNodes = 0;
  std::size_t numElements = 0;

  std::vector<double> xCoordinates;
  std::vector<double> yCoordinates;
  std::vector<double> topobathy;
  std::vector<int> landMask;
  std::vector<int> faceNodeConnectivity;

  /**
   * @brief Access non-owning span of longitude/x coordinates.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getXCoordinates() const noexcept {
    return xCoordinates.empty() ? shared.getXCoordinates() : xCoordinates;
  }

  /**
   * @brief Access non-owning span of latitude/y coordinates.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getYCoordinates() const noexcept {
    return yCoordinates.empty() ? shared.getYCoordinates() : yCoordinates;
  }

  /**
   * @brief Access non-owning span of topobathy elevation.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getTopobathy() const noexcept {
    return topobathy.empty() ? shared.getTopobathy() : topobathy;
  }

  /**
   * @brief Access non-owning span of land/sea mask.
   * @return std::span<const int>
   */
  [[nodiscard]] std::span<const int> getLandMask() const noexcept {
    return landMask.empty() ? shared.getLandMask() : landMask;
  }

  /**
   * @brief Access non-owning span of element face-node connectivity.
   * @return std::span<const int>
   */
  [[nodiscard]] std::span<const int> getFaceNodeConnectivity() const noexcept {
    if (!faceNodeConnectivity.empty()) {
      return faceNodeConnectivity;
    }
    if (const auto *tri = std::get_if<TriangularGridData>(&specific)) {
      return tri->getFaceNodeConnectivity();
    }
    return {};
  }

  /**
   * @brief Check if grid contains rectangular specific data.
   * @return bool
   */
  [[nodiscard]] bool isRectangular() const noexcept {
    return std::holds_alternative<RectangularGridData>(specific) ||
           gridType == GridType::Rectangular ||
           shared.gridType == GridType::Rectangular;
  }

  /**
   * @brief Check if grid contains triangular specific data.
   * @return bool
   */
  [[nodiscard]] bool isTriangular() const noexcept {
    return std::holds_alternative<TriangularGridData>(specific) ||
           gridType == GridType::Triangular ||
           shared.gridType == GridType::Triangular;
  }

  /**
   * @brief Check if grid contains SMC specific data.
   * @return bool
   */
  [[nodiscard]] bool isSMC() const noexcept {
    return std::holds_alternative<SMCGridData>(specific) ||
           gridType == GridType::SMC || shared.gridType == GridType::SMC;
  }

  /**
   * @brief Access pointer to rectangular grid specific data if active.
   * @return const RectangularGridData*
   */
  [[nodiscard]] const RectangularGridData *getRectangularData() const noexcept {
    return std::get_if<RectangularGridData>(&specific);
  }

  /**
   * @brief Access pointer to triangular mesh specific data if active.
   * @return const TriangularGridData*
   */
  [[nodiscard]] const TriangularGridData *getTriangularData() const noexcept {
    return std::get_if<TriangularGridData>(&specific);
  }

  /**
   * @brief Access pointer to SMC grid specific data if active.
   * @return const SMCGridData*
   */
  [[nodiscard]] const SMCGridData *getSMCData() const noexcept {
    return std::get_if<SMCGridData>(&specific);
  }

  /**
   * @brief Resets all full grid fields and arrays to default empty state.
   */
  void clear() noexcept;
};

// --- LocalGridPartition -----------------------------------------------------
/**
 * @struct LocalGridPartition
 * @brief Sub-domain local compute grid for MPI execution or local partitions.
 * @details Encapsulates local sub-mesh dimensions, local coordinate vectors,
 *          contiguous local topobathy buffers, local land/sea mask, halo cell
 *          counts, and local-to-global cell index mappings.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-21
 * @date Last update : 2026-10-21
 */
struct LocalGridPartition {
  SharedGridData shared;
  SpecificGridData specific;

  // --- Convenience member aliases for backward compatibility ---
  GridType gridType = GridType::Undefined;
  int rankId = 0;

  std::size_t localNx = 0;
  std::size_t localNy = 0;
  std::size_t localNumNodes = 0;

  std::size_t haloSizeX = 0;
  std::size_t haloSizeY = 0;

  std::vector<double> xCoordinates;
  std::vector<double> yCoordinates;
  std::vector<double> topobathy;
  std::vector<int> landMask;
  std::vector<std::size_t> localToGlobalMap;

  /**
   * @brief Access non-owning span of local longitude/x coordinates.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getXCoordinates() const noexcept {
    return xCoordinates.empty() ? shared.getXCoordinates() : xCoordinates;
  }

  /**
   * @brief Access non-owning span of local latitude/y coordinates.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getYCoordinates() const noexcept {
    return yCoordinates.empty() ? shared.getYCoordinates() : yCoordinates;
  }

  /**
   * @brief Access non-owning span of local topobathy elevation.
   * @return std::span<const double>
   */
  [[nodiscard]] std::span<const double> getTopobathy() const noexcept {
    return topobathy.empty() ? shared.getTopobathy() : topobathy;
  }

  /**
   * @brief Access non-owning span of local land/sea mask.
   * @return std::span<const int>
   */
  [[nodiscard]] std::span<const int> getLandMask() const noexcept {
    return landMask.empty() ? shared.getLandMask() : landMask;
  }

  /**
   * @brief Access non-owning span of local-to-global cell index mapping.
   * @return std::span<const std::size_t>
   */
  [[nodiscard]] std::span<const std::size_t>
  getLocalToGlobalMap() const noexcept {
    return localToGlobalMap;
  }

  /**
   * @brief Check if partition contains rectangular specific data.
   * @return bool
   */
  [[nodiscard]] bool isRectangular() const noexcept {
    return std::holds_alternative<RectangularGridData>(specific) ||
           gridType == GridType::Rectangular ||
           shared.gridType == GridType::Rectangular;
  }

  /**
   * @brief Check if partition contains triangular specific data.
   * @return bool
   */
  [[nodiscard]] bool isTriangular() const noexcept {
    return std::holds_alternative<TriangularGridData>(specific) ||
           gridType == GridType::Triangular ||
           shared.gridType == GridType::Triangular;
  }

  /**
   * @brief Check if partition contains SMC specific data.
   * @return bool
   */
  [[nodiscard]] bool isSMC() const noexcept {
    return std::holds_alternative<SMCGridData>(specific) ||
           gridType == GridType::SMC || shared.gridType == GridType::SMC;
  }

  /**
   * @brief Resets all local partition fields and arrays to default empty state.
   */
  void clear() noexcept;
};

} // namespace ww4_utils
