// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UPolityHistoryItemEntry.h"
#include "CoreMinimal.h"

class BURIN_API UPolityDataEntry
{
public:
	UPolityDataEntry();
	~UPolityDataEntry();

	FString name;
	TArray<uint8_t> mapcolor1;
	TArray<uint8_t> mapcolor2;
	TArray<uint8_t> mapcolor3;
	TArray<UPolityHistoryItemEntry> history;
};
