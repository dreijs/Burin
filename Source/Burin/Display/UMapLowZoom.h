// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Data/Earth/UTerrain.h"
#include "UTriangleDataEntry.h"
#include "UEdgeDataEntry.h"
#include "FLineDisplayData.h"

class BURIN_API UMapLowZoom
{

public:
	// Sets default values for this actor's properties
	UMapLowZoom();
	~UMapLowZoom();

	int GetTerrainDataAtCoordinate(UTerrain* terrain, double x, double y);

	FString GetTerrainText(UTerrain* terrain, int v);

	TArray<FCanvasUVTri> GetTriangles(UTerrain* terrain, int mode);

	TArray<FLineDisplayData> GetBorders(int mode);

	void Initialize();

private:
	TArray<UTriangleDataEntry> TriangleData;
	TArray<UEdgeDataEntry> EdgeData;
};
