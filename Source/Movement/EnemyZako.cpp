// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyZako.h"
#include "Components/CapsuleComponent.h"
#include "ServantAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Weapon.h"
#include "AIManager.h"
#include "MyTypes.h"


AEnemyZako::AEnemyZako()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> meshObject1(TEXT("/Game/Mannequin/Character/Mesh/SK_Mannequin"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> animInstance1(TEXT("/Game/EnemyAnim/EnemyAnimBP"));

	GetMesh()->SetSkeletalMesh(meshObject1.Object);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetAnimInstanceClass(animInstance1.Class);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -97), FRotator(0, -90, 0));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECollisionResponse::ECR_Ignore);

	isPlayer = false;
	invincible = true;
	doNothing = true;

	Tags.Add("Targetable");
}

void AEnemyZako::BeginPlay()
{
	Super::Super::Super::BeginPlay();

	desiredStandoff = 170;

	armor = 0;

	GetCharacterMovement()->MaxWalkSpeed = (800);

	currentMode = (FMath::RandBool() ? EESM_Attack : EESM_Wait);
	timeToCycleMode = FMath::FRandRange(0, 5);
	animInstance = Cast<UServantAnimInstance>(GetMesh()->GetAnimInstance());

	InitWeaponForEnemy();
}

void AEnemyZako::SetRefPlayer(AServant* p)
{
	playerCharacter = p;
	if (!playerCharacter)
	{
		TArray<AActor*> foundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AServant::StaticClass(), foundActors);

		for (const auto foundActor : foundActors)
		{
			AServant* otherChar = Cast<AServant>(foundActor);
			if (otherChar->GetIsPlayer())
			{
				playerCharacter = otherChar;
				break;
			}
		}
	}
}

void AEnemyZako::Tick(float DeltaTime)
{
	Super::Super::Tick(DeltaTime);

	//if (movementRestricted) IMyTypes::Debug_Print(FString::FromInt(movementRestricted));

	if (inSelfDestroySequence)
	{
		HandleSelfDestroySequence(DeltaTime);
		return;
	}
	if (forceMovementControl) ProcessMovementControl(DeltaTime);

	if (doNothing) return;

	
	HandleModeCycle(DeltaTime);
	if (currentMode == EESnemyMode::EESM_Attack)
	{
		if (readyForNextAttack)
		{
			HandleAttackMode();
		}
	}

	MoveToTarget();
}

#pragma region Mode

void AEnemyZako::ForceEnemyMode(EESnemyMode forceMode)
{
	timeToCycleMode = FMath::FRandRange(10, 15.f);
	currentMode = forceMode;
}

void AEnemyZako::HandleModeCycle(float DeltaTime)
{
	timeToCycleMode -= DeltaTime;
	if (timeToCycleMode < 0)
	{
		CycleMode();
	}
}

void AEnemyZako::CycleMode()
{
	if (currentMode == EESM_Attack)
	{
		currentMode = EESM_Wait;
		rnd = FMath::RandBool() ? 1 : -1;
		timeToCycleMode = FMath::FRandRange(1.f, 5.f);
	}
	else if (currentMode == EESM_Wait)
	{
		currentMode = EESM_Attack;
		timeToCycleMode = FMath::FRandRange(3.f, 10.f);
	}

}

#pragma endregion

#pragma region Combat

void AEnemyZako::InitWeaponForEnemy()
{
	int weaponIndex = FMath::RandRange(0, 5);

	TSoftClassPtr<AWeapon> weaponClass;

	switch (weaponIndex)
	{
	case 0:
		weaponClass = TSoftClassPtr<AWeapon>(FSoftObjectPath(TEXT("Blueprint'/Game/Weapons/Sword.Sword_C'")));
		break;
	case 1:
		weaponClass = TSoftClassPtr<AWeapon>(FSoftObjectPath(TEXT("Blueprint'/Game/Weapons/Spear.Spear_C'")));
		break;
	case 2:
		weaponClass = TSoftClassPtr<AWeapon>(FSoftObjectPath(TEXT("Blueprint'/Game/Weapons/Hammer.Hammer_C'")));
		break;
	case 3:
		weaponClass = TSoftClassPtr<AWeapon>(FSoftObjectPath(TEXT("Blueprint'/Game/Weapons/Dagger.Dagger_C'")));
		break;
	case 4:
		weaponClass = TSoftClassPtr<AWeapon>(FSoftObjectPath(TEXT("Blueprint'/Game/Weapons/BigAxe.BigAxe_C'")));
		break;
	case 5:
		weaponClass = TSoftClassPtr<AWeapon>(FSoftObjectPath(TEXT("Blueprint'/Game/Weapons/Mace.Mace_C'")));
		break;
	default:
		break;
	}

	if (weaponClass)
	{
		UClass* myPtr = weaponClass.LoadSynchronous();

		FActorSpawnParameters myP = FActorSpawnParameters();
		myP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		personalWeapon = Cast<AWeapon>(GetWorld()->SpawnActor<AWeapon>(myPtr, FVector::ZeroVector, FRotator(0, 0, 0), myP));
		personalWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "right_hand_socket");
		personalWeapon->OrientSelfToWeapon(2);
		personalWeapon->InitHeldMode(this);
		personalWeapon->canScan = true;
		personalWeapon->weaponDamage = .01;
	}
	else
	{
		IMyTypes::Debug_Print("Failed to init weapon", 10);
	}

}

void AEnemyZako::HandleAttackMode()
{

	FVector dirToTarget = playerCharacter->GetActorLocation() - GetActorLocation();
	float distanceToTarget = dirToTarget.Size();

	if (distanceToTarget < desiredStandoff)
	{
		TryAttack();
	}
}

void AEnemyZako::TryAttack()
{
	animInstance->AttackCommand(false, 1);
}

void AEnemyZako::DoDamage(float dmgIn)
{
	if (invincible || noHit) return;

	if (health < 0 || dmgIn <= 0) return;

	health -= dmgIn;

	if (health < 0)
	{
		CharacterDestroyByDamage();
	}
}

void AEnemyZako::CharacterDestroyByDamage()
{
	inSelfDestroySequence = true;

	Tags.Empty();

	FVector dirToFly = GetActorForwardVector();

	if (playerCharacter)
	{
		if (playerCharacter->GetLockedTarget() == this)
		{
			playerCharacter->ReportLockedTargetDestroyed();
		}
		dirToFly = playerCharacter->GetActorLocation() - GetActorLocation();
		dirToFly.Normalize();
	}


	if (personalWeapon) personalWeapon->DoWeaponDropAndRemove();
	if (aiManager) aiManager->ReportEnemyDestroyed(this);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(IMyTypes::Weapon, ECR_Ignore);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetPhysicsLinearVelocity(-1 * dirToFly * 10000 * FMath::FRandRange(.5, 1.256));
}

void AEnemyZako::ProcessEnemyHitEvent(AActor* otherActor)
{
	if (!otherActor) return;
	DoHitOther(otherActor);

	float damage = .05 * FMath::FRandRange(.1, 2) * comboMult;
	Cast<AServant>(otherActor)->DoTakeHit(currentAttackSpecialEffect);
	Cast<AServant>(otherActor)->DoDamage(damage);

	//IMyTypes::Debug_Print(damage, 10, 10);
}

#pragma endregion

#pragma region movement

#pragma endregion