// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class BURIN_API UPlaceHistoryItemEntry
{
public:
	UPlaceHistoryItemEntry();
	~UPlaceHistoryItemEntry();

	FString name;
	double latitude, longitude;
};
