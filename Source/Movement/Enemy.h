// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UCLASS()
class MOVEMENT_API AEnemy : public ACharacter
{
	GENERATED_BODY()



public:
	// Sets default values for this character's properties
	typedef enum EEnemyMode
	{
		EEM_Wait = 0,
		EEM_Attack = 1,
	};

	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	class UServantAnimInstance* animInstance;

#pragma region Servant reference

	class AServant* playerCharacter;

#pragma endregion

#pragma region enemymode

	void HandleModeCycle(float DeltaTime);
	void CycleMode();
	float timeToCycleMode = 30.f;
	void ForceEnemyMode(EEnemyMode forceMode);

	EEnemyMode currentMode = EEnemyMode::EEM_Wait;

#pragma endregion

#pragma region movement

	UFUNCTION(BlueprintCallable)
	void SetMovementRestricted(bool isMoveRestrict, bool isAimRestrict, bool isDodgeRestrict);

	bool isMoveRestricted = false;
	void MoveToTarget();


#pragma endregion


#pragma region combat

	class AWeapon* personalWeapon;

	void HandleAttackMode();
	void TryAttack();

	UFUNCTION(BlueprintCallable)
	void StartAttackEvent();
	UFUNCTION(BlueprintCallable)
	void EndAttackEvent();

#pragma endregion

};
