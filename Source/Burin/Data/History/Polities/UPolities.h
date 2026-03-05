// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UPolityDataEntry.h"

class BURIN_API UPolities
{
public:

	UPolities();
	~UPolities();

	TArray<UPolityDataEntry> PolityData;

	void InitializePolities();

private:
	TMap<FString, int32> polityMap;
};
