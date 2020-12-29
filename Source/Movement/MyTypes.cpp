// Fill out your copyright notice in the Description page of Project Settings.

#include "MyTypes.h"
#include "Engine.h"


// Add default functionality here for any IMyTypes functions that are not pure virtual.


void IMyTypes::PrintList(TArray<FString> &inList)
{
	if (inList.Num() > 0)
	{
		int key = 0;

		for (const auto str : inList)
		{

		}
	}
}

void IMyTypes::Debug_Print(const FString &inStr, int printTime, int key)
{
	GEngine->AddOnScreenDebugMessage(key, printTime, FColor::Emerald, inStr, true);
}

void IMyTypes::Debug_Print(const float inVal, int printTime, int key)
{
	GEngine->AddOnScreenDebugMessage(key, printTime, FColor::Emerald, FString::SanitizeFloat(inVal), true);
}

void IMyTypes::Debug_Print(const int inVal, int printTime, int key)
{
	GEngine->AddOnScreenDebugMessage(key, printTime, FColor::Emerald, FString::FromInt(inVal), true);
}