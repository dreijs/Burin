// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Data/Earth/UTerrain.h"
#include "UTriangleDataEntry.h"
#include "UEdgeDataEntry.h"
#include "UPointDataEntry.h"
#include "FLineDisplayData.h"
#include <Burin/Concepts/Provinces/UProvince.h>

class BURIN_API UMapLowZoom
{

public:
	// Sets default values for this actor's properties
	UMapLowZoom();
	~UMapLowZoom();

	int GetTerrainDataAtCoordinate(UTerrain* terrain, int zoomCategory, double lon, double lat);
	int GetTriangleIDAtCoordinate(int zoomCategory, double lon, double lat);

	FString GetTerrainText(UTerrain* terrain, int v);

	TArray<FCanvasUVTri> GetTriangles(UTerrain* terrain, int mode, int zoomCategory, double minLat, double minLon, double maxLat, double maxLon, int offsetX, int offsetY, int width, int height);
	TArray<FCanvasUVTri> GetMaterialTriangles(UTerrain* terrain, int mode, int zoomCategory, int x, int y);
	TArray<FLineDisplayData> GetBorders(int mode, int zoomCategory, int x, int y);
	TArray<FLineDisplayData> GetRivers(int mode, int zoomCategory, int x, int y);
	TArray<FCanvasUVTri> GetProvinceTriangles(TArray<UProvince> provinces, int mode, int zoomCategory, int x, int y);

	TArray<int> GetSubregionIndices(int zoomCategory, double lat, double lon, double latDelta, double lonDelta);
	int GetNumSubregions(int zoomCategory, bool isX);

	void Initialize();

private:
	// for each zoom level, for each x coordinate, for each y coordinate, a list of triangle/point/edge data
	TArray < TArray < TArray < TArray <UTriangleDataEntry> > > > TriangleData;
	TArray < TArray < TArray < TArray <UEdgeDataEntry> > > > EdgeData;
	TArray < TArray < TArray < TArray <UPointDataEntry> > > > PointData;

	//TArray < TArray <UTriangleDataEntry> > TriangleData;
	//TArray < TArray <UEdgeDataEntry> > EdgeData;
	//TArray < TArray <UPointDataEntry> > PointData;

	//TArray <UTriangleDataEntry> TriangleData;
	//TArray <UEdgeDataEntry> EdgeData;
	//TArray <UPointDataEntry> PointData;

	UPointDataEntry getFirstPoint(bool b, int edge, int zoomCategory, int x, int y);
};
