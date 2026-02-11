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

	int x1, y1, x2, y2, x3, y3;
	int biomeData;
};
