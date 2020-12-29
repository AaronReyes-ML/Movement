// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "NobleAurora.generated.h"

/**
 * 
 */
UCLASS()
class MOVEMENT_API ANobleAurora : public AWeapon
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void DoAuroraAOE(UParticleSystem* aoeSys);

};
