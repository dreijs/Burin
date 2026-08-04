// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Canvas.h"
#include "Burin/Data/Earth/FTerrain.h"
#include "FTriangleDataEntry.h"
#include "FEdgeDataEntry.h"
#include "FPointDataEntry.h"
#include "FLineDisplayData.h"
#include "Burin/Concepts/Provinces/FPlace.h"

class BURIN_API FMapLowZoom
{

public:
	int32 GetTerrainDataAtCoordinate(FTerrain* terrain, int32 zoomCategory, double lon, double lat);
	int32 GetTriangleIDAtCoordinate(int32 zoomCategory, double lon, double lat);

	FString GetTerrainText(FTerrain* terrain, int32 v);

	TArray<FCanvasUVTri> GetTriangles(FTerrain* terrain, int32 mode, int32 zoomCategory, double minLat, double minLon, double maxLat, double maxLon, int32 offsetX, int32 offsetY, int32 width, int32 height);
	TArray<FCanvasUVTri> GetMaterialTriangles(FTerrain* terrain, int32 mode, int32 zoomCategory, int32 x, int32 y);
	TArray<FLineDisplayData> GetBorders(int32 mode, int32 zoomCategory, int32 x, int32 y);
	TArray<FLineDisplayData> GetRivers(int32 mode, int32 zoomCategory, int32 x, int32 y);
	TArray<FCanvasUVTri> GetProvinceTriangles(const TArray<FPlace>& provinces, int32 mode, int32 zoomCategory, int32 x, int32 y);

	TArray<int32> GetSubregionIndices(int32 zoomCategory, double lat, double lon, double latDelta, double lonDelta);
	int32 GetNumSubregions(int32 zoomCategory, bool isX);

	void Initialize();

private:
	// for each zoom level, for each x coordinate, for each y coordinate, a list of triangle/point/edge data
	TArray < TArray < TArray < TArray <FTriangleDataEntry> > > > TriangleData;
	TArray < TArray < TArray < TArray <FEdgeDataEntry> > > > EdgeData;
	TArray < TArray < TArray < TArray <FPointDataEntry> > > > PointData;

	FPointDataEntry GetFirstPoint(bool b, int32 edge, int32 zoomCategory, int32 x, int32 y);
	TArray<FCanvasUVTri>& AddBordersAsTriangles(TArray<FCanvasUVTri>& result, int32 zoomCategory, int32 x, int32 y, double minX, double maxX, double minY, double maxY, int32 offsetX, int32 offsetY, int32 width, int32 height);
};
