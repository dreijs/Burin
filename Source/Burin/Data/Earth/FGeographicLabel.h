// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *
 */
struct FGeographicLabel
{
	FString Name;
	double Latitude = 0.0;
	double Longitude = 0.0;
	FString Type;
	int32 Importance = 0;
	int32 MinYear = 0;
	int32 MaxYear = 0;
	TArray<uint8> Color;
};
