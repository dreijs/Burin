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
};
