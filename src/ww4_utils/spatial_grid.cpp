/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file spatial_grid.cpp
 * @brief Implementation of spatial grid utility methods in WAVEWATCH IV.
 * @details Implements memory reset routines for SharedGridData,
 *          RectangularGridData, TriangularGridData, SMCGridData, FullGrid,
 *          and LocalGridPartition.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-21
 * @date Last update : 2026-10-21
 */

#include "ww4_utils/spatial_grid.h"

namespace ww4_utils {

// --- SharedGridData ---------------------------------------------------------

void SharedGridData::clear() noexcept {
  gridType = GridType::Undefined;
  name.clear();
  spatialReferenceSystem.clear();
  isSpherical = true;
  numNodes = 0;
  numElements = 0;
  xCoordinates.clear();
  yCoordinates.clear();
  topobathy.clear();
  landMask.clear();
  obstructionX.clear();
  obstructionY.clear();
  mapsta.clear();
}

// --- RectangularGridData ----------------------------------------------------

void RectangularGridData::clear() noexcept {
  nx = 0;
  ny = 0;
  x0 = 0.0;
  y0 = 0.0;
  dx = 0.0;
  dy = 0.0;
  xGridSpacing.clear();
  yGridSpacing.clear();
  isGlobal = false;
}

// --- TriangularGridData -----------------------------------------------------

void TriangularGridData::clear() noexcept {
  meshTopologyName.clear();
  nodeCoordinatesNames.clear();
  faceNodeConnectivityName.clear();
  numEdges = 0;
  faceNodeConnectivity.clear();
  edgeNodeConnectivity.clear();
  faceFaceConnectivity.clear();
  faceEdgeConnectivity.clear();
  elementAreas.clear();
  edgeLengths.clear();
}

// --- SMCGridData ------------------------------------------------------------

void SMCGridData::clear() noexcept {
  baseNx = 0;
  baseNy = 0;
  numActiveCells = 0;
  numIFaces = 0;
  numJFaces = 0;
  cells.clear();
  iFaceLinkage.clear();
  jFaceLinkage.clear();
}

// --- FullGrid ---------------------------------------------------------------

void FullGrid::clear() noexcept {
  shared.clear();
  specific = std::monostate{};
  gridType = GridType::Undefined;
  name.clear();
  nx = 0;
  ny = 0;
  numNodes = 0;
  numElements = 0;
  xCoordinates.clear();
  yCoordinates.clear();
  topobathy.clear();
  landMask.clear();
  faceNodeConnectivity.clear();
}

// --- LocalGridPartition -----------------------------------------------------

void LocalGridPartition::clear() noexcept {
  shared.clear();
  specific = std::monostate{};
  gridType = GridType::Undefined;
  rankId = 0;
  localNx = 0;
  localNy = 0;
  localNumNodes = 0;
  haloSizeX = 0;
  haloSizeY = 0;
  xCoordinates.clear();
  yCoordinates.clear();
  topobathy.clear();
  landMask.clear();
  localToGlobalMap.clear();
}

} // namespace ww4_utils
