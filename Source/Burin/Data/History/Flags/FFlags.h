// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FFlagDataEntry.h"

/**
 *
 */
class BURIN_API FFlags
{
public:

	TArray<FFlagDataEntry> FlagData;

	void InitializeFlags();

private:
	TMap<FString, int32> FlagMap;
};
