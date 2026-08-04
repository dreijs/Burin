// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPolityHistoryItemEntry.h"

struct FPolityDataEntry
{
	FString Name;
	TArray<uint8> MapColor1;
	TArray<uint8> MapColor2;
	TArray<uint8> MapColor3;
	TArray<FPolityHistoryItemEntry> History;
};
