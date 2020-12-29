// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIManager.generated.h"

UCLASS()
class MOVEMENT_API AAIManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAIManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	class AServant* player;

	TArray<class AServant*> activeEnemies;

	void InitializeEnemies();

	int activeServantCount = 0;
	bool servantActive = false;

	UPROPERTY(EditAnywhere)
	float spawnServantChance = -1;
	UPROPERTY(EditAnywhere)
	int maxActiveServants = 1;
	UPROPERTY(EditAnywhere)
	int maxActiveEnemies = 5;
	UPROPERTY(EditAnywhere)
	bool initAsInvincible = false;
	UPROPERTY(EditAnywhere)
	bool initAsDoNothing = false;

	float spawnDelay = 0;
	void AddEnemy(bool ignoreSpawnDelay);
	void RemoveEnemy(class AServant* toRemove);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	class AServant* GetRefPlayer() { return player; };

	void ReportEnemyDestroyed(class AServant* enemyDestroyed);

	void StopSpawning();

};
