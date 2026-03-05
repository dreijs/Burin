// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class BURIN_API UColorDataEntry
{
public:
	UColorDataEntry();
	~UColorDataEntry();	
	
	FString name;
	TArray<uint8_t> color;
};
