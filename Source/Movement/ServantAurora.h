// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyServant.h"
#include "ServantAurora.generated.h"

/**
 * 
 */
UCLASS()
class MOVEMENT_API AServantAurora : public AEnemyServant
{
	GENERATED_BODY()


	AServantAurora();
public:
	UFUNCTION(BlueprintCallable)
	void DoAuroraIceSpawn();

	virtual void ReadyForNextAttack(bool r) override;

protected:

	UPROPERTY(EditAnywhere)
	USkeletalMesh* sourceMesh;

	int HIT_1 = 1;
	int HIT_2 = 2;
	int HIT_3 = 3;
	int HIT_4 = 4;
	int CHARGE = 5;
	int RING = 6;
	int JUMP = 7;
	int ICE_SPAWN = 8;
	int DIVE = 9;

	UClass* aurora_weaponClass = nullptr;

	TArray<int> currentComboList;
	bool nextResetInitWait = false;
	int currentComboIndex = 0;

	TArray<TArray<int>> comboList;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void InitWeaponForEnemy() override;
	virtual void HandleAttackMode() override;
	virtual void ResetCombo();

	UParticleSystem* spawnedIceParticles;

};
