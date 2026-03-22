// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class BURIN_API UPolity
{

public:
	UPolity();
	~UPolity();

	FString Name;	
	TArray<uint8_t> mapcolor1;
	TArray<uint8_t> mapcolor2;
	TArray<uint8_t> mapcolor3;
};
