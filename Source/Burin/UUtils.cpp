// Fill out your copyright notice in the Description page of Project Settings.


#include "UUtils.h"

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

bool UUtils::ExtractBoolFromXMLContentLine(FString aString) {
	TArray<FString> stringArray1 = {}, stringArray2 = {};
	aString.ParseIntoArray(stringArray1, TEXT(">"), false);
	if (stringArray1.Num() >= 2) {
		stringArray1[1].ParseIntoArray(stringArray2, TEXT("<"), false);
		if (stringArray2[0].Contains("true")) return true;
		if (stringArray2[0].Contains("yes")) return true;
		return false;
	}

	UE_LOG(LogTemp, Error, TEXT("Bad string passed to ExtractIntFromXMLContentLine: %s"), *aString);
	return false;
}

int32 UUtils::ExtractIntFromXMLContentLine(FString aString) {
	TArray<FString> stringArray1 = {}, stringArray2 = {};
	aString.ParseIntoArray(stringArray1, TEXT(">"), false);
	if (stringArray1.Num() >= 2) {
		stringArray1[1].ParseIntoArray(stringArray2, TEXT("<"), false);
		return FCString::Atoi(*stringArray2[0]);
	}

	UE_LOG(LogTemp, Error, TEXT("Bad string passed to ExtractIntFromXMLContentLine: %s"), *aString);
	return -1;
}

double UUtils::ExtractDoubleFromXMLContentLine(FString aString) {
	TArray<FString> stringArray1 = {}, stringArray2 = {};
	aString.ParseIntoArray(stringArray1, TEXT(">"), false);
	if (stringArray1.Num() >= 2) {
		stringArray1[1].ParseIntoArray(stringArray2, TEXT("<"), false);
		return FCString::Atod(*stringArray2[0]);
	}

	UE_LOG(LogTemp, Error, TEXT("Bad string passed to ExtractDoubleFromXMLContentLine: %s"), *aString);
	return -1;
}
TArray<uint8> UUtils::ExtractUInt8ArrayFromXMLContentLine(FString aString) {
	FString bString = UUtils::ExtractStringFromXMLContentLine(aString);
	TArray<FString> stringArray1 = {}, stringArray2 = {};
	bString.ParseIntoArray(stringArray1, TEXT(","), false);
	TArray<uint8> result = {};
	if (stringArray1.Num() >= 0) {
		for (int32 i = 0; i < stringArray1.Num(); i++) {
			result.Add(FCString::Atoi(*stringArray1[i]));
		}
		return result;
	}
	UE_LOG(LogTemp, Error, TEXT("Bad string passed to ExtractUInt8ArrayFromXMLContentLine: %s"), *aString);
	return { };
}

TArray<double> UUtils::ExtractDoubleArrayFromXMLContentLine(FString aString) {
	FString bString = UUtils::ExtractStringFromXMLContentLine(aString);
	TArray<FString> stringArray1 = {}, stringArray2 = {};
	bString.ParseIntoArray(stringArray1, TEXT(","), false);
	TArray<double> result = {};
	if (stringArray1.Num() >= 0) {
		for (int32 i = 0; i < stringArray1.Num(); i++) {
			result.Add(FCString::Atod(*stringArray1[i]));
		}
		return result;
	}
	UE_LOG(LogTemp, Error, TEXT("Bad string passed to ExtractDoubleArrayFromXMLContentLine: %s"), *aString);
	return { };
}
