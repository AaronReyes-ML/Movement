// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ClimbAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MOVEMENT_API UClimbAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetClimbing(bool climb);

	UFUNCTION(BlueprintImplementableEvent)
	void DoClamber();

	UFUNCTION(BlueprintImplementableEvent)
	void Climb(FVector dir);

	UFUNCTION(BlueprintImplementableEvent)
	void ReportPositions(FHitResult &rightHand, FHitResult &leftHand, FHitResult &rightFoot, FHitResult &leftFoot);
	
};
