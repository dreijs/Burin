// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * A piece of a mesh triangle that the domain radius cut through. Its corners lie on or inside the
 * triangle it was cut from, so a fragment never strays outside its parent and never overlaps a
 * neighbouring triangle's pieces.
 */
struct FDomainFragment
{
	// Index into the Places array that owns this piece.
	int32 Place = INDEX_NONE;

	// The mesh triangle this piece was cut from. Pieces are stored in ascending triangle order, so
	// drawing can interleave them with the whole triangles and keep the mesh's own draw order --
	// the mesh overlaps itself in places, and there the higher triangle index is meant to win.
	int32 Triangle = INDEX_NONE;

	// The piece's three corners, in mesh degrees.
	FVector2D A = FVector2D::ZeroVector;
	FVector2D B = FVector2D::ZeroVector;
	FVector2D C = FVector2D::ZeroVector;
};

/**
 * One tile's worth of domain assignment, produced by FMapLowZoom::BuildPlaceDomains().
 *
 * Most triangles fall wholly inside one place's domain and are recorded in Owner alone. The few the
 * radius passes through are marked in bClipped and contribute their pieces to Fragments instead, so
 * that a domain boundary can run through the middle of a triangle rather than having to take or
 * leave the whole thing -- which matters because the ear-clipped mesh contains triangles hundreds
 * of kilometres long.
 */
struct FTileDomains
{
	// Per triangle in the tile: index into the Places array that owns it, or INDEX_NONE.
	TArray<int32> Owner;

	// Per triangle: true when the radius cut through it, so the domain holds only the pieces of it
	// listed in Fragments rather than the whole triangle.
	TArray<bool> bClipped;

	// Explicit geometry for the cut triangles, in no particular order.
	TArray<FDomainFragment> Fragments;

	// Per place, the colour its domain is painted. Resolved once when the domains are built, so
	// that drawing them needs no access to the places or the polities. A place whose owner never
	// resolved to a known polity is left at zero alpha and goes unpainted.
	TArray<FColor> PlaceColor;

	void Reset(int32 numTriangles, int32 numPlaces)
	{
		Owner.Init(INDEX_NONE, numTriangles);
		bClipped.Init(false, numTriangles);
		Fragments.Reset();
		PlaceColor.Init(FColor(0, 0, 0, 0), numPlaces);
	}
};
