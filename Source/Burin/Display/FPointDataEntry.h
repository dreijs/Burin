// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *
 */
struct FPointDataEntry
{
	double X = 0.0;
	double Y = 0.0;
	TArray<int32> Neighbors;
};
