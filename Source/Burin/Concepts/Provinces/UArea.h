// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class BURIN_API UArea
{

public:
	UArea();
	~UArea();

	FString name;
	FString commonName;
	double latitude;
	double longitude;
};
