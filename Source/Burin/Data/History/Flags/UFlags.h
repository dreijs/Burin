// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UFlagDataEntry.h"

/**
 *
 */
class BURIN_API UFlags
{
public:

	UFlags();
	~UFlags();

	TArray<UFlagDataEntry> FlagData;

	void InitializeFlags();

private:
	TMap<FString, int32> flagMap;
};
