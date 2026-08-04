// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UUtils.generated.h"

UCLASS(Blueprintable)
class BURIN_API UUtils : public UObject
{
	GENERATED_BODY()

public:
	static FString ExtractStringFromXMLContentLine(FString aString);
	static bool ExtractBoolFromXMLContentLine(FString aString);
	static int32 ExtractIntFromXMLContentLine(FString aString);
	static double ExtractDoubleFromXMLContentLine(FString aString);
	static TArray<uint8> ExtractUInt8ArrayFromXMLContentLine(FString aString);
	static TArray<double> ExtractDoubleArrayFromXMLContentLine(FString aString);
};
