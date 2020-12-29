// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ServantAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MOVEMENT_API UServantAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
	
public:

	UFUNCTION(BlueprintImplementableEvent)
	void SetGameMode(int mode);

	UFUNCTION(BlueprintImplementableEvent)
	void DoSearch(bool search);

	UFUNCTION(BlueprintImplementableEvent)
	void SetAimAngle(float pitch);

	UFUNCTION(BlueprintImplementableEvent)
	void Do_TraceLine(bool search);

	UFUNCTION(BlueprintImplementableEvent)
	void TraceOn_DiscardOnly();

	UFUNCTION(BlueprintImplementableEvent)
	void TraceOn_FirstDiscard();

	UFUNCTION(BlueprintImplementableEvent)
	void TraceOn_Normal();

	UFUNCTION(BlueprintImplementableEvent)
	void TraceOn_Noble();

	UFUNCTION(BlueprintImplementableEvent)
	void TraceOn_Init();
	UFUNCTION(BlueprintImplementableEvent)
	void TraceOn_Cancel();

	UFUNCTION(BlueprintImplementableEvent)
	void AttackCommand(bool ignoreBuffer, int bufferCode);

	UFUNCTION(BlueprintImplementableEvent)
	void HoldAttackCommand();

	UFUNCTION(BlueprintImplementableEvent)
	void SetMovementAnimations(const class UAnimSequence* idleAnim, const class UBlendSpace* movementAnim, const class UAnimMontage* traceOnAnim);

	UFUNCTION(BlueprintImplementableEvent)
	void SetAttackMontage(const class UAnimMontage* inMontage);

	UFUNCTION(BlueprintImplementableEvent)
	void SetSprint(bool sprint);

	UFUNCTION(BlueprintImplementableEvent)
	void ReportMovementInput(bool newMovementInput);

	UFUNCTION(BlueprintImplementableEvent)
	void Dodge(int type);

	UFUNCTION(BlueprintImplementableEvent)
	void WeaponDestroyed();

	UFUNCTION(BlueprintImplementableEvent)
	void TraceOn_Noble_Break();

	UFUNCTION(BlueprintImplementableEvent)
	void TraceOn_Noble_Break_Launch();

	UFUNCTION(BlueprintImplementableEvent)
	void SetDesiredNextAttack(const FString &nextAttack);

	UFUNCTION(BlueprintImplementableEvent)
	void DoDamageEffect(const FString &specialEffect);

	UFUNCTION(BlueprintImplementableEvent)
	void TraceMode(bool on);


	UFUNCTION(BlueprintImplementableEvent)
	void StartStopClimb(bool climb);
	UFUNCTION(BlueprintImplementableEvent)
	void SetClimbDirection(const FVector2D &dirToClimb);

	UFUNCTION(BlueprintImplementableEvent)
	void SetBaseIKLocators(const FVector &rightHand, const FVector &leftHand, const FVector &rightFoot, const FVector &leftFoot);

	UFUNCTION(BlueprintImplementableEvent)
	void JumpFromWall();

	UFUNCTION(BlueprintImplementableEvent)
	void Clamber();
};
