// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Servant.h"
#include "Components/CapsuleComponent.h"
#include "ServantAnimInstance.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Weapon.h"
#include "MyTypes.h"


#pragma region construct

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> meshObject1(TEXT("/Game/Mannequin/Character/Mesh/SK_Mannequin"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> animInstance1(TEXT("/Game/EnemyAnim/EnemyAnimBP"));

	GetMesh()->SetSkeletalMesh(meshObject1.Object);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetAnimInstanceClass(animInstance1.Class);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -97), FRotator(0, -90, 0));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);


}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();


	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AServant::StaticClass(), foundActors);

	if (foundActors.Num() > 0)
	{
		AServant* otherChar = Cast<AServant>(foundActors[0]);
		if (otherChar)
		{
			playerCharacter = otherChar;
			IMyTypes::Debug_Print("Found servant", 10);
		}
		else
		{
			IMyTypes::Debug_Print("Failed to find servant actor", 10);
		}
	}
	else
	{
		IMyTypes::Debug_Print("Failed to find servant actor", 10);
	}

	currentMode = EEM_Attack;
	animInstance = Cast<UServantAnimInstance>(GetMesh()->GetAnimInstance());
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
}

#pragma endregion

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleModeCycle(DeltaTime);

	if (currentMode == EEnemyMode::EEM_Attack)
	{
		HandleAttackMode();
	}
	
}

#pragma region Combat

void AEnemy::HandleAttackMode()
{
	MoveToTarget();

	FVector dirToTarget = playerCharacter->GetActorLocation() - GetActorLocation();
	float distanceToTarget = dirToTarget.Size();

	IMyTypes::Debug_Print(distanceToTarget, 1, 0);

	if (distanceToTarget < 100)
	{
		TryAttack();
	}
}

void AEnemy::TryAttack()
{
	IMyTypes::Debug_Print(IsValid(animInstance) ? "V" : "B", 2, 10);
	animInstance->AttackCommand(false, 1);
}

void AEnemy::StartAttackEvent()
{

}

void AEnemy::EndAttackEvent()
{

}

#pragma endregion

#pragma region Mode

void AEnemy::ForceEnemyMode(EEnemyMode forceMode)
{
	timeToCycleMode = FMath::FRandRange(10, 15.f);
	currentMode = forceMode;
}

void AEnemy::HandleModeCycle(float DeltaTime)
{
	timeToCycleMode -= DeltaTime;
	if (timeToCycleMode < 0)
	{
		CycleMode();
	}
}

void AEnemy::CycleMode()
{
	if (currentMode == EEM_Attack)
	{
		currentMode = EEM_Wait;
		timeToCycleMode = FMath::FRandRange(3.f, 6.f);
	}
	else if (currentMode == EEM_Wait)
	{ 
		currentMode = EEM_Attack;
		timeToCycleMode = FMath::FRandRange(10.f, 45.f);
	}

}

#pragma endregion

#pragma region movement

void AEnemy::SetMovementRestricted(bool isMoveRestrict, bool isAimRestrict, bool isDodgeRestrict)
{
	isMoveRestricted = isMoveRestrict;
}

void AEnemy::MoveToTarget()
{
	if (isMoveRestricted) return;

	FVector dirToTarget = playerCharacter->GetActorLocation() - GetActorLocation();
	SetActorRotation(FMath::QInterpTo(GetActorQuat(),(dirToTarget * FVector(1,1,0)).ToOrientationQuat(), GetWorld()->GetDeltaSeconds(), 1));
	AddMovementInput(dirToTarget, 1);
}

#pragma endregion

