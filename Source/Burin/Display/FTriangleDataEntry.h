// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *
 */
struct FTriangleDataEntry
{
	int32 E1 = -1;
	int32 E2 = -1;
	int32 E3 = -1;
	bool bB1 = false;
	bool bB2 = false;
	bool bB3 = false;
	int32 TerrainData = 0;

	// Triangles on other layers covering the same ground as this one. A region's polygon is its
	// outer boundary filled solid and a region enclosed within it is drawn on top rather than cut
	// out, so the two overlap and their shared boundary has a triangle on one side only -- T2 goes
	// negative and neither can see the other through the edges. These are the links that were
	// missing: without them a flood fill walking the layer underneath can never step up, which at
	// level 1 left 71,594 km2 of Canada unreachable from its own mainland.
	TArray<int32> CrossLayer;

	// How much of this triangle sits beneath later-drawn ones, in square kilometres.
	// Subtract it before adding a triangle's area into a total, or ground under an overlay gets
	// counted once for each layer -- 0.90% of land overall, and up to 4.8% for a single biome.
	double CoveredArea = 0.0;
};
