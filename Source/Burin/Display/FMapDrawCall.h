// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FMapDrawCall.generated.h"

/**
 * One tile to draw, at one zoom category, into one rectangle of a render target.
 *
 * Everything a draw needs, worked out in C++ so the caller wires nothing by hand. Three separate
 * quantities have to agree for a tile to appear in the right place, and each of them has to be
 * derived from the zoom category rather than written down:
 *
 *   - the tile grid          how many tiles the category has, from GetNumSubregions()
 *   - the mesh-degree box    which ground the tile covers, as the view fractions below
 *   - the destination rect   which pixels of the render target it is drawn into
 *
 * Holding any one of them as a constant works until a second category starts using the same code,
 * and then fails silently: the wrong grid draws a quarter of the world, the wrong box draws each
 * tile displaced by its own index, and the wrong rectangle draws everything somewhere invisible.
 * All three are computed together here, from the same category, so they cannot drift apart.
 */
USTRUCT(BlueprintType)
struct FMapDrawCall
{
	GENERATED_BODY()

	/** Which level's mesh to read. */
	UPROPERTY(BlueprintReadOnly, Category = "RenderMap")
	int32 ZoomCategory = 0;

	UPROPERTY(BlueprintReadOnly, Category = "RenderMap")
	int32 TileX = 0;

	UPROPERTY(BlueprintReadOnly, Category = "RenderMap")
	int32 TileY = 0;

	// The destination rectangle on the render target, in pixels. Adjacent tiles share an edge
	// exactly -- the rectangles are computed as differences of rounded boundaries rather than as a
	// rounded width each, so rounding cannot open a one-pixel seam between them.
	UPROPERTY(BlueprintReadOnly, Category = "RenderMap")
	int32 OffsetX = 0;

	UPROPERTY(BlueprintReadOnly, Category = "RenderMap")
	int32 OffsetY = 0;

	UPROPERTY(BlueprintReadOnly, Category = "RenderMap")
	int32 Width = 0;

	UPROPERTY(BlueprintReadOnly, Category = "RenderMap")
	int32 Height = 0;

	// The view fractions the existing accessors take, covering exactly this tile. Passing these
	// with this ZoomCategory makes GetTerrainTriangles() resolve to this same TileX/TileY.
	UPROPERTY(BlueprintReadOnly, Category = "RenderMap")
	double MinFracX = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "RenderMap")
	double MinFracY = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "RenderMap")
	double MaxFracX = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "RenderMap")
	double MaxFracY = 0.0;
};
