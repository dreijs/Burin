// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FPlace
{
	FString Name;
	FString CommonName;
	double Latitude = 0.0;
	double Longitude = 0.0;

	// The triangle this place stands on, into the level-1 tile's TriangleData -- or the nearest
	// land triangle when the place falls just offshore of the simplified coastline. INDEX_NONE
	// when there is no land near it at all (a small island level 1 doesn't carry), rather than a
	// -1 that would be indexed into the mesh by mistake.
	//
	// Level 1 only. Triangle ids mean nothing across levels, so BuildPlaceDomains() finds its own
	// seed in whichever level's mesh it is flooding -- which is also why a place INDEX_NONE here
	// can still hold a domain at level 3, where its island exists.
	int32 SeedTriangle = INDEX_NONE;

	// This place's domain: the triangles grown outward from its seed by
	// FMapLowZoom::BuildPlaceDomains(), in ascending triangle order. Empty until that has run,
	// and empty afterwards for a place with no seed or one whose seed a nearer place took.
	//
	// Global triangle ids (see FMapLowZoom::ToGlobalTriangle) into the level that was built last,
	// which with lazy per-level building is not necessarily level 1. Bookkeeping only: it is not
	// what the domains are drawn or measured from, both of which read the level's own owner map,
	// and it names only the places a contested triangle is nearest to.
	//
	// A triangle the domain radius passes through is listed here too, even though only part of it
	// belongs to the place; FTileDomains holds the pieces that are actually inside.
	TArray<int32> Triangles;

	// Index into UBurinWorld::Polities, or INDEX_NONE when the place's owner didn't resolve to
	// a known polity. Interned rather than holding an FPolity copy so that "do these two places
	// share a polity?" is an integer compare in per-edge and per-triangle loops, and so a
	// polity's colors have exactly one home.
	int32 ControllerIndex = INDEX_NONE;
};
