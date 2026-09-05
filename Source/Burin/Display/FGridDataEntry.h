// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * Precomputed spatial index for one tile's triangles, built offline by the map generator and
 * loaded verbatim from LevelData.bin. Cells is a uniform grid over the tile's local pixel space,
 * row-major (index = cellY * Width + cellX); each cell lists the indices (into that tile's
 * TriangleData) of the triangles whose bounding box overlaps it, so a coordinate lookup only
 * needs to test one cell's candidates instead of every triangle in the tile.
 */
struct FGridDataEntry
{
	int32 Width = 0;
	int32 Height = 0;

	TArray<TArray<int32>> Cells;
};
