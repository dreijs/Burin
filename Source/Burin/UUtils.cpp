// Fill out your copyright notice in the Description page of Project Settings.


#include "UUtils.h"

UUtils::UUtils()
{
}

UUtils::~UUtils()
{
}

FString UUtils::ExtractStringFromXMLContentLine(FString aString) {
	TArray<FString> stringArray1 = {}, stringArray2 = {};
	aString.ParseIntoArray(stringArray1, TEXT(">"), false);
	if (stringArray1.Num() >= 2) {
		stringArray1[1].ParseIntoArray(stringArray2, TEXT("<"), false);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Bad string passed to ExtractStringFromXMLContentLine: %s"), *aString);
		return "";
	}
	return stringArray2[0];
}

TArray<uint8_t> UUtils::ExtractUInt8ArrayFromXMLContentLine(FString aString) {
	FString bString = UUtils::ExtractStringFromXMLContentLine(aString);
	TArray<FString> stringArray1 = {}, stringArray2 = {};
	bString.ParseIntoArray(stringArray1, TEXT(","), false);
	TArray<uint8_t> result = {};
	if (stringArray1.Num() >= 0) {
		for (int i = 0; i < stringArray1.Num(); i++) {
			result.Add(FCString::Atoi(*stringArray1[i]));
		}
		return result;
	}
	UE_LOG(LogTemp, Error, TEXT("Bad string passed to ExtractUInt8ArrayFromXMLContentLine: %s"), *aString);
	return { };
}

TArray<double> UUtils::ExtractFloatArrayFromXMLContentLine(FString aString) {
	FString bString = UUtils::ExtractStringFromXMLContentLine(aString);
	TArray<FString> stringArray1 = {}, stringArray2 = {};
	bString.ParseIntoArray(stringArray1, TEXT(","), false);
	TArray<double> result = {};
	if (stringArray1.Num() >= 0) {
		for (int i = 0; i < stringArray1.Num(); i++) {
			result.Add(FCString::Atof(*stringArray1[i]));
		}
		return result;
	}
	UE_LOG(LogTemp, Error, TEXT("Bad string passed to ExtractFloatArrayFromXMLContentLine: %s"), *aString);
	return { };
}