// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Burin/Concepts/Polities/FPolity.h"

struct FPlace
{
	FString Name;
	FString CommonName;
	double Latitude = 0.0;
	double Longitude = 0.0;
	TArray<int32> Triangles;
	FPolity Controller;
};
