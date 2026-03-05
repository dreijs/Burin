// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class BURIN_API UPlaceDataEntry
{
public:
	UPlaceDataEntry();
	~UPlaceDataEntry();

	FString name;
	FString commonName;
	double latitude, longitude;
};
