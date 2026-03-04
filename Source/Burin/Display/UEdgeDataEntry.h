// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class BURIN_API UEdgeDataEntry
{
public:
	UEdgeDataEntry();
	~UEdgeDataEntry();

	int x1, y1, x2, y2;
	int t1, t2;
	int riverData;
};
