// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServantUI.generated.h"

/**
 * 
 */
UCLASS()
class MOVEMENT_API UServantUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent)
	void AddWeapon(class AWeapon* weapon);
	
	UFUNCTION(BlueprintImplementableEvent)
	void RemoveWeapon(class AWeapon* weapon);

	UFUNCTION(BlueprintImplementableEvent)
	void SetCrosshair(FVector2D loc);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSelected(UClass* selected, int index);

	UFUNCTION(BlueprintImplementableEvent)
	void EnemyDefeat(int current, int max);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHealth(float health);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateSpecPercent(float spec);
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCrestPercent(float crest);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowLocOnMark(bool show);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateLocPosition(const FVector &worldLoc);
};
