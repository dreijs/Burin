// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Burin/Concepts/Polities/UPolity.h>

class BURIN_API UProvince
{

public:
	UProvince();
	~UProvince();

	FString Name;
	FString CommonName;
	double Latitude, Longitude;
	TArray<int> Triangles;
	UPolity controller;
};
