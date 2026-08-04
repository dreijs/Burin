// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *
 */
struct FTerrainDataEntry
{
	FString Name;
	TArray<uint8> Color;
	TArray<TArray<TArray<int32>>> Conditions;
};
