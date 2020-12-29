// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ServantGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MOVEMENT_API UServantGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	TArray<TSubclassOf<class AWeapon>*> nobleCache;
	
};
