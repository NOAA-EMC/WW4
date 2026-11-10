/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_spatial_grid.cpp
 * @brief Unit tests for NetCDF-UGRID spatial grid data structures.
 * @details Uses Google Test to verify SharedGridData, RectangularGridData,
 *          TriangularGridData, SMCCell, SMCGridData, FullGrid, and
 *          LocalGridPartition construction, accessors, spans, variants,
 *          and clear routines.
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
#include <gtest/gtest.h>

namespace ww4_utils {

// --- SharedGridData tests ----------------------------------------------------

TEST(SpatialGridTest, SharedGridDataDefaultsAndClear) {
  SharedGridData shared;
  EXPECT_EQ(shared.gridType, GridType::Undefined);
  EXPECT_TRUE(shared.name.empty());
  EXPECT_TRUE(shared.spatialReferenceSystem.empty());
  EXPECT_TRUE(shared.isSpherical);
  EXPECT_EQ(shared.numNodes, 0);
  EXPECT_EQ(shared.numElements, 0);

  EXPECT_TRUE(shared.getXCoordinates().empty());
  EXPECT_TRUE(shared.getYCoordinates().empty());
  EXPECT_TRUE(shared.getTopobathy().empty());
  EXPECT_TRUE(shared.getLandMask().empty());
  EXPECT_TRUE(shared.getObstructionX().empty());
  EXPECT_TRUE(shared.getObstructionY().empty());
  EXPECT_TRUE(shared.getMapsta().empty());

  shared.gridType = GridType::Rectangular;
  shared.name = "shared_mesh";
  shared.spatialReferenceSystem = "EPSG:4326";
  shared.isSpherical = true;
  shared.numNodes = 3;
  shared.numElements = 2;
  shared.xCoordinates = {0.0, 1.0, 2.0};
  shared.yCoordinates = {10.0, 11.0, 12.0};
  shared.topobathy = {100.0, 50.0, -10.0};
  shared.landMask = {1, 1, 0};
  shared.obstructionX = {1.0, 0.8, 0.0};
  shared.obstructionY = {1.0, 0.9, 0.0};
  shared.mapsta = {1, 1, 0};

  EXPECT_EQ(shared.getXCoordinates().size(), 3);
  EXPECT_EQ(shared.getYCoordinates().size(), 3);
  EXPECT_EQ(shared.getTopobathy().size(), 3);
  EXPECT_EQ(shared.getLandMask().size(), 3);
  EXPECT_EQ(shared.getObstructionX().size(), 3);
  EXPECT_EQ(shared.getObstructionY().size(), 3);
  EXPECT_EQ(shared.getMapsta().size(), 3);

  shared.clear();
  EXPECT_EQ(shared.gridType, GridType::Undefined);
  EXPECT_TRUE(shared.name.empty());
  EXPECT_TRUE(shared.getXCoordinates().empty());
  EXPECT_TRUE(shared.getTopobathy().empty());
}

// --- RectangularGridData tests -----------------------------------------------

TEST(SpatialGridTest, RectangularGridDataDefaultsAndClear) {
  RectangularGridData rect;
  EXPECT_EQ(rect.nx, 0);
  EXPECT_EQ(rect.ny, 0);
  EXPECT_DOUBLE_EQ(rect.x0, 0.0);
  EXPECT_DOUBLE_EQ(rect.y0, 0.0);
  EXPECT_DOUBLE_EQ(rect.dx, 0.0);
  EXPECT_DOUBLE_EQ(rect.dy, 0.0);
  EXPECT_FALSE(rect.isGlobal);
  EXPECT_TRUE(rect.getXGridSpacing().empty());
  EXPECT_TRUE(rect.getYGridSpacing().empty());

  rect.nx = 360;
  rect.ny = 180;
  rect.x0 = 0.0;
  rect.y0 = -90.0;
  rect.dx = 1.0;
  rect.dy = 1.0;
  rect.isGlobal = true;
  rect.xGridSpacing = {1.0, 1.0, 1.0};
  rect.yGridSpacing = {1.0, 1.0, 1.0};

  EXPECT_EQ(rect.nx, 360);
  EXPECT_TRUE(rect.isGlobal);
  EXPECT_EQ(rect.getXGridSpacing().size(), 3);
  EXPECT_EQ(rect.getYGridSpacing().size(), 3);

  rect.clear();
  EXPECT_EQ(rect.nx, 0);
  EXPECT_FALSE(rect.isGlobal);
  EXPECT_TRUE(rect.getXGridSpacing().empty());
}

// --- TriangularGridData tests -------------------------------------------

TEST(SpatialGridTest, TriangularGridDataDefaultsAndClear) {
  TriangularGridData tri;
  EXPECT_TRUE(tri.meshTopologyName.empty());
  EXPECT_TRUE(tri.nodeCoordinatesNames.empty());
  EXPECT_TRUE(tri.faceNodeConnectivityName.empty());
  EXPECT_EQ(tri.numEdges, 0);
  EXPECT_TRUE(tri.getFaceNodeConnectivity().empty());
  EXPECT_TRUE(tri.getEdgeNodeConnectivity().empty());
  EXPECT_TRUE(tri.getFaceFaceConnectivity().empty());
  EXPECT_TRUE(tri.getFaceEdgeConnectivity().empty());
  EXPECT_TRUE(tri.getElementAreas().empty());
  EXPECT_TRUE(tri.getEdgeLengths().empty());

  tri.meshTopologyName = "mesh_topology";
  tri.nodeCoordinatesNames = "node_x node_y";
  tri.faceNodeConnectivityName = "face_node_connectivity";
  tri.numEdges = 3;
  tri.faceNodeConnectivity = {0, 1, 2};
  tri.edgeNodeConnectivity = {0, 1, 1, 2, 2, 0};
  tri.faceFaceConnectivity = {-1, -1, -1};
  tri.faceEdgeConnectivity = {0, 1, 2};
  tri.elementAreas = {1000.0};
  tri.edgeLengths = {100.0, 100.0, 100.0};

  EXPECT_EQ(tri.getFaceNodeConnectivity().size(), 3);
  EXPECT_EQ(tri.getEdgeNodeConnectivity().size(), 6);
  EXPECT_EQ(tri.getFaceFaceConnectivity().size(), 3);
  EXPECT_EQ(tri.getFaceEdgeConnectivity().size(), 3);
  EXPECT_EQ(tri.getElementAreas().size(), 1);
  EXPECT_EQ(tri.getEdgeLengths().size(), 3);

  tri.clear();
  EXPECT_TRUE(tri.meshTopologyName.empty());
  EXPECT_EQ(tri.numEdges, 0);
  EXPECT_TRUE(tri.getFaceNodeConnectivity().empty());
  EXPECT_TRUE(tri.getElementAreas().empty());
}

// --- SMCGridData & SMCCell tests ---------------------------------------------

TEST(SpatialGridTest, SMCGridDataDefaultsAndClear) {
  SMCGridData smc;
  EXPECT_EQ(smc.baseNx, 0);
  EXPECT_EQ(smc.baseNy, 0);
  EXPECT_EQ(smc.numActiveCells, 0);
  EXPECT_EQ(smc.numIFaces, 0);
  EXPECT_EQ(smc.numJFaces, 0);
  EXPECT_TRUE(smc.getCells().empty());
  EXPECT_TRUE(smc.getIFaceLinkage().empty());
  EXPECT_TRUE(smc.getJFaceLinkage().empty());

  SMCCell cell1{0, 10, 20, 1, 0.25, 0.25, 1.0};
  SMCCell cell2{1, 11, 20, 1, 0.25, 0.25, 1.0};

  smc.baseNx = 1440;
  smc.baseNy = 721;
  smc.numActiveCells = 2;
  smc.numIFaces = 4;
  smc.numJFaces = 4;
  smc.cells = {cell1, cell2};
  smc.iFaceLinkage = {0, 1, 2, 3};
  smc.jFaceLinkage = {0, 1, 2, 3};

  EXPECT_EQ(smc.getCells().size(), 2);
  EXPECT_EQ(smc.getCells()[0].cellId, 0);
  EXPECT_EQ(smc.getIFaceLinkage().size(), 4);
  EXPECT_EQ(smc.getJFaceLinkage().size(), 4);

  smc.clear();
  EXPECT_EQ(smc.baseNx, 0);
  EXPECT_EQ(smc.numActiveCells, 0);
  EXPECT_TRUE(smc.getCells().empty());
  EXPECT_TRUE(smc.getIFaceLinkage().empty());
}

// --- FullGrid tests ----------------------------------------------------------

TEST(SpatialGridTest, FullGridDefaultsAndClear) {
  FullGrid grid;
  EXPECT_EQ(grid.gridType, GridType::Undefined);
  EXPECT_TRUE(grid.name.empty());
  EXPECT_EQ(grid.nx, 0);
  EXPECT_EQ(grid.ny, 0);
  EXPECT_EQ(grid.numNodes, 0);
  EXPECT_EQ(grid.numElements, 0);
  EXPECT_TRUE(grid.getXCoordinates().empty());
  EXPECT_TRUE(grid.getYCoordinates().empty());
  EXPECT_TRUE(grid.getTopobathy().empty());
  EXPECT_TRUE(grid.getLandMask().empty());
  EXPECT_TRUE(grid.getFaceNodeConnectivity().empty());

  grid.gridType = GridType::Rectangular;
  grid.name = "global_025deg";
  grid.nx = 1440;
  grid.ny = 721;
  grid.numNodes = 1440 * 721;
  grid.xCoordinates = {0.0, 0.25, 0.5};
  grid.yCoordinates = {-90.0, -89.75, -89.5};
  grid.topobathy = {100.0, 50.0, -10.0};
  grid.landMask = {1, 1, 0};
  grid.faceNodeConnectivity = {0, 1, 2};

  EXPECT_EQ(grid.gridType, GridType::Rectangular);
  EXPECT_EQ(grid.name, "global_025deg");
  EXPECT_EQ(grid.getXCoordinates().size(), 3);
  EXPECT_EQ(grid.getYCoordinates().size(), 3);
  EXPECT_EQ(grid.getTopobathy().size(), 3);
  EXPECT_EQ(grid.getLandMask().size(), 3);
  EXPECT_EQ(grid.getFaceNodeConnectivity().size(), 3);
  EXPECT_TRUE(grid.isRectangular());
  EXPECT_FALSE(grid.isTriangular());
  EXPECT_FALSE(grid.isSMC());

  grid.clear();
  EXPECT_EQ(grid.gridType, GridType::Undefined);
  EXPECT_TRUE(grid.name.empty());
  EXPECT_EQ(grid.nx, 0);
  EXPECT_EQ(grid.ny, 0);
  EXPECT_TRUE(grid.getXCoordinates().empty());
  EXPECT_TRUE(grid.getTopobathy().empty());
}

TEST(SpatialGridTest, FullGridWithSpecificVariants) {
  FullGrid grid;
  grid.shared.gridType = GridType::Rectangular;
  grid.shared.name = "rect_shared";
  grid.shared.xCoordinates = {0.0, 1.0};
  grid.shared.yCoordinates = {0.0, 1.0};

  RectangularGridData rdata;
  rdata.nx = 2;
  rdata.ny = 2;
  rdata.dx = 1.0;
  rdata.dy = 1.0;
  rdata.isGlobal = false;
  grid.specific = rdata;

  EXPECT_TRUE(grid.isRectangular());
  EXPECT_NE(grid.getRectangularData(), nullptr);
  EXPECT_EQ(grid.getRectangularData()->nx, 2);
  EXPECT_EQ(grid.getTriangularData(), nullptr);
  EXPECT_EQ(grid.getSMCData(), nullptr);

  // Switch to Triangular
  grid.shared.gridType = GridType::Triangular;
  TriangularGridData tdata;
  tdata.meshTopologyName = "mesh2d";
  tdata.faceNodeConnectivity = {0, 1, 2};
  grid.specific = tdata;

  EXPECT_TRUE(grid.isTriangular());
  EXPECT_NE(grid.getTriangularData(), nullptr);
  EXPECT_EQ(grid.getTriangularData()->meshTopologyName, "mesh2d");
  EXPECT_EQ(grid.getFaceNodeConnectivity().size(), 3);

  // Switch to SMC
  grid.shared.gridType = GridType::SMC;
  SMCGridData sdata;
  sdata.baseNx = 720;
  sdata.baseNy = 360;
  grid.specific = sdata;

  EXPECT_TRUE(grid.isSMC());
  EXPECT_NE(grid.getSMCData(), nullptr);
  EXPECT_EQ(grid.getSMCData()->baseNx, 720);
}

// --- LocalGridPartition tests ------------------------------------------------

TEST(SpatialGridTest, LocalGridPartitionDefaultsAndClear) {
  LocalGridPartition part;
  EXPECT_EQ(part.gridType, GridType::Undefined);
  EXPECT_EQ(part.rankId, 0);
  EXPECT_EQ(part.localNx, 0);
  EXPECT_EQ(part.localNy, 0);
  EXPECT_EQ(part.localNumNodes, 0);
  EXPECT_EQ(part.haloSizeX, 0);
  EXPECT_EQ(part.haloSizeY, 0);
  EXPECT_TRUE(part.getXCoordinates().empty());
  EXPECT_TRUE(part.getYCoordinates().empty());
  EXPECT_TRUE(part.getTopobathy().empty());
  EXPECT_TRUE(part.getLandMask().empty());
  EXPECT_TRUE(part.getLocalToGlobalMap().empty());

  part.gridType = GridType::Triangular;
  part.rankId = 2;
  part.localNumNodes = 100;
  part.haloSizeX = 2;
  part.xCoordinates = {10.0, 11.0};
  part.yCoordinates = {20.0, 21.0};
  part.topobathy = {300.0, -50.0};
  part.landMask = {1, 0};
  part.localToGlobalMap = {10, 11};

  EXPECT_EQ(part.gridType, GridType::Triangular);
  EXPECT_EQ(part.rankId, 2);
  EXPECT_EQ(part.getXCoordinates().size(), 2);
  EXPECT_EQ(part.getTopobathy().size(), 2);
  EXPECT_EQ(part.getLocalToGlobalMap().size(), 2);
  EXPECT_EQ(part.getLocalToGlobalMap()[0], 10);
  EXPECT_TRUE(part.isTriangular());

  part.clear();
  EXPECT_EQ(part.gridType, GridType::Undefined);
  EXPECT_EQ(part.rankId, 0);
  EXPECT_TRUE(part.getXCoordinates().empty());
  EXPECT_TRUE(part.getLocalToGlobalMap().empty());
}

} // namespace ww4_utils
