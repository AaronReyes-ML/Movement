// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Servant.h"
#include "EnemyServant.generated.h"

/**
 * 
 */
UCLASS()
class MOVEMENT_API AEnemyServant : public AServant
{
	GENERATED_BODY()
	
public:
	typedef enum EESnemyMode
	{
		EESM_Wait = 0,
		EESM_Attack = 1,
	};

	virtual void SetRefPlayer(class AServant* p);

	AEnemyServant();

	bool doNothing = false;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	bool readyForNextAttack = true;

	class AServant* playerCharacter;

	virtual void InitWeaponForEnemy();

	virtual void CharacterDestroyByDamage() override;

#pragma region mode

	virtual void HandleModeCycle(float DeltaTime);
	virtual void CycleMode();
	float timeToCycleMode = 30.f;
	virtual void ForceEnemyMode(EESnemyMode forceMode);
	EESnemyMode currentMode = EESnemyMode::EESM_Wait;

#pragma endregion

#pragma region movement
	int rnd = 1;
	virtual void MoveToTarget();

	float desiredStandoff = 300;
	float softBoundRegionSize = 50;
	bool isInOverrun = false;


#pragma endregion

#pragma region combat

	bool doingcombo = false;
	bool finishcombowithspecial = false;
	int comboCounter = 0;
	int maxCombo = 0;
	float attackWait = 0;
	FString specialAttackCode = "0";
	void OrientDirectToTarget();
	virtual void HandleAttackMode();
	virtual void TryAttack();

	virtual void ResetCombo();

	virtual void DoDamage(float dmgToDo) override;


#pragma endregion

public:
	UFUNCTION(BlueprintCallable)
	virtual void ReadyForNextAttack(bool r);
};
