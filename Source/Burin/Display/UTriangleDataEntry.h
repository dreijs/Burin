// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class BURIN_API UTriangleDataEntry
{
public:
	UTriangleDataEntry();
	~UTriangleDataEntry();

	int e1, e2, e3;
	bool b1, b2, b3;
	int terrainData;
};
