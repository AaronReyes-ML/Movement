// Fill out your copyright notice in the Description page of Project Settings.


#include "AIManager.h"
#include "Servant.h"
#include "EnemyServant.h"
#include "ServantAurora.h"
#include "EnemyZako.h"
#include "MyTypes.h"
#include "Classes/Kismet/GameplayStatics.h"

// Sets default values
AAIManager::AAIManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickInterval(.25);
}

// Called when the game starts or when spawned
void AAIManager::BeginPlay()
{
	Super::BeginPlay();

	InitializeEnemies();
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AServant::StaticClass(), foundActors);

	for (const auto foundActor : foundActors)
	{
		AServant* otherChar = Cast<AServant>(foundActor);
		if (otherChar->GetIsPlayer())
		{
			player = otherChar;
			break;
		}
	}

}

// Called every frame
void AAIManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (spawnDelay > 0) spawnDelay -= DeltaTime;
	else
	{
		if (activeEnemies.Num() < maxActiveEnemies)
		{
			AddEnemy(false);
		}
	}
}

void AAIManager::InitializeEnemies()
{
	for (int i = 0; i < maxActiveEnemies; i++)
	{
		AddEnemy(true);
	}
}

void AAIManager::AddEnemy(bool ignoreSpawnDelay)
{
	if (!ignoreSpawnDelay && spawnDelay > 0)
	{
		return;
	}

	if (activeEnemies.Num() < maxActiveEnemies)
	{

		FActorSpawnParameters myP = FActorSpawnParameters();
		myP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AServant* spawned = nullptr;
		bool spawnServ = (FMath::RandRange(0, 100) <= spawnServantChance * 100);

		if (spawnServ && activeServantCount < maxActiveServants)
		{
			activeServantCount++;
			spawned = Cast<AServant>(GetWorld()->SpawnActor<AEnemyServant>(AServantAurora::StaticClass(), FVector(FMath::RandRange(-3000, 3000), FMath::RandRange(-3000, 3000), 115), FRotator::ZeroRotator, myP));
		}
		else
		{
			spawned = Cast<AServant>(GetWorld()->SpawnActor<AEnemyZako>(AEnemyZako::StaticClass(), FVector(FMath::RandRange(-3000, 3000), FMath::RandRange(-3000, 3000), 115), FRotator::ZeroRotator, myP));
		}

		if (spawned)
		{
			activeEnemies.Add(spawned);
			spawned->SpawnDefaultController();
			spawned->aiManager = this;

			Cast<AEnemyServant>(spawned)->SetRefPlayer(player);
			if (!initAsInvincible) spawned->SetIsInvincible(false);
			if (!initAsDoNothing) Cast<AEnemyServant>(spawned)->doNothing = false;

			spawnDelay = 1;
		}
		else
		{
			if (activeEnemies.Num() < maxActiveEnemies) AddEnemy(true);
		}
	}
}

void AAIManager::RemoveEnemy(AServant* enemyToRemove)
{
	if (!enemyToRemove) return;

	activeEnemies.RemoveSingle(enemyToRemove);
	activeEnemies.Shrink();

	if (activeEnemies.Num() < maxActiveEnemies) AddEnemy(false);
	else
	{
		IMyTypes::Debug_Print("Fail spawn", 50);
	}
}

void AAIManager::ReportEnemyDestroyed(AServant* destroyedEnemy)
{
	if (!destroyedEnemy) return;
	RemoveEnemy(destroyedEnemy);

	if (!player) return;


	if (!destroyedEnemy->IsA(AEnemyZako::StaticClass()))
	{
		activeServantCount--;
		player->ReportEnemyDestroyed(true);
	}
	else
	{
		player->ReportEnemyDestroyed(false);
	}

}

void AAIManager::StopSpawning()
{

}