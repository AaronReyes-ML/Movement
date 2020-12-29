// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyServant.h"
#include "EnemyZako.generated.h"

/**
 * 
 */
UCLASS()
class MOVEMENT_API AEnemyZako : public AEnemyServant
{
	GENERATED_BODY()

public:

	virtual void SetRefPlayer(class AServant* p);
	AEnemyZako();

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void InitWeaponForEnemy();
	virtual void CharacterDestroyByDamage() override;
	virtual void ProcessEnemyHitEvent(AActor* otherActor) override;

#pragma region mode

	virtual void HandleModeCycle(float DeltaTime);
	virtual void CycleMode();

	virtual void ForceEnemyMode(EESnemyMode forceMode);

#pragma endregion

#pragma region movement

#pragma endregion

#pragma region combat

	virtual void HandleAttackMode();
	virtual void TryAttack();
	virtual void DoDamage(float dmgToDo) override;

#pragma endregion

};
