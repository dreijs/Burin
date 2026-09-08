// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *
 */
struct FTerrainDataEntry
{
	FString Name;

	// The map palette, from <displaycolor0>. Built to make categories legible: saturated, well
	// separated, high contrast. Used by every mode that wants to be read rather than believed.
	TArray<uint8> Color;

	// The photographic palette, from <photocolor>. What the same ground looks like from orbit,
	// which is close to the opposite: desaturated, narrow-ranged, lifted by haze. Falls back to
	// Color for an entry that has no <photocolor>, so a partial table still loads.
	TArray<uint8> PhotoColor;

	TArray<TArray<TArray<int32>>> Conditions;
};
