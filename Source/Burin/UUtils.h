// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UUtils.generated.h"

UCLASS(Blueprintable)
class BURIN_API UUtils : public UObject
{
	GENERATED_BODY()

public:
	UUtils();
	~UUtils();

	static FString ExtractStringFromXMLContentLine(FString aString);
	static TArray<uint8_t> ExtractUInt8ArrayFromXMLContentLine(FString aString);
	static TArray<double> ExtractFloatArrayFromXMLContentLine(FString aString);

private:
	void InitializeTerrain();
};
