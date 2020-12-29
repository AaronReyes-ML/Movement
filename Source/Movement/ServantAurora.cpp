// Fill out your copyright notice in the Description page of Project Settings.


#include "ServantAurora.h"
#include "Components/CapsuleComponent.h"
#include "ServantAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Weapon.h"
#include "Particles/ParticleSystem.h"
#include "MyTypes.h"

AServantAurora::AServantAurora()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> asset1(TEXT("ParticleSystem'/Game/ParagonAurora/FX/Particles/Abilities/Leap/FX/P_Aurora_Decoy_Frost.P_Aurora_Decoy_Frost'"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> animInstance1(TEXT("/Game/EnemyServantAnim/Aurora/Servant_Aurora.Servant_Aurora_C"));

	if (!sourceMesh)
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> meshObject1(TEXT("SkeletalMesh'/Game/ParagonAurora/Characters/Heroes/Aurora/Meshes/Aurora.Aurora'"));
		sourceMesh = meshObject1.Object;
	}

	GetMesh()->SetSkeletalMesh(sourceMesh);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetAnimInstanceClass(animInstance1.Class);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -97), FRotator(0, -90, 0));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECollisionResponse::ECR_Ignore);

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	if (!spawnedIceParticles) spawnedIceParticles = asset1.Object;

	isPlayer = false;
	invincible = true;
	doNothing = true;

	Tags.Add("Targetable");
}

void AServantAurora::BeginPlay()
{
	Super::Super::Super::BeginPlay();

	health = 10;
	armor = 1;

	GetCharacterMovement()->MaxWalkSpeed = (650);

	currentMode = EESM_Attack;
	timeToCycleMode = 20;
	animInstance = Cast<UServantAnimInstance>(GetMesh()->GetAnimInstance());

	InitWeaponForEnemy();

	TArray<int> combo1 = { HIT_1, HIT_2, HIT_3, HIT_4 };
	TArray<int> combo2 = { RING, HIT_1, HIT_4};
	TArray<int> combo3 = { HIT_3, HIT_2, CHARGE};
	TArray<int> combo4 = { HIT_1, HIT_2, CHARGE, CHARGE };
	TArray<int> combo5 = { HIT_1, JUMP};
	TArray<int> combo6 = { HIT_3, HIT_2, ICE_SPAWN };
	TArray<int> combo7 = { HIT_1, HIT_2, RING };
	TArray<int> combo8 = { ICE_SPAWN, HIT_2, HIT_3, CHARGE };
	TArray<int> combo9 = { HIT_2, HIT_1, CHARGE, JUMP};

	comboList = { combo1, combo2, combo3, combo4, combo5, combo6, combo7, combo8, combo9 };
}

void AServantAurora::Tick(float DeltaTime)
{
	Super::Super::Tick(DeltaTime);

	/*
	if (comboList.Num() > 0)
	{
		IMyTypes::Debug_Print(FString::SanitizeFloat(attackWait) + " - Current combo: " + FString::FromInt(currentComboIndex) + ". " +
			FString::FromInt(comboCounter) + "/" + FString::FromInt(maxCombo), 10, 1);
	}
	*/

	if (activeAEOOverlaps.Num() > 0) HandleAOEDespawn(DeltaTime);

	if (inSelfDestroySequence)
	{
		HandleSelfDestroySequence(DeltaTime);
		return;
	}
	if (forceMovementControl) ProcessMovementControl(DeltaTime);

	if (doNothing) return;

	if (attackWait > 0) attackWait -= DeltaTime;

	HandleModeCycle(DeltaTime);
	if (currentMode == EESM_Attack)
	{
		HandleAttackMode();
	}

	MoveToTarget();
}

#pragma region combat

void AServantAurora::InitWeaponForEnemy()
{
	int weaponIndex = FMath::RandRange(0, 5);

	TSoftClassPtr<AWeapon> weaponClass = TSoftClassPtr<AWeapon>(FSoftObjectPath(TEXT("Blueprint'/Game/Weapons/Noble/MyNobleAurora.MyNobleAurora_C'")));;

	if (weaponClass)
	{
		UClass* myPtr = weaponClass.LoadSynchronous();
		aurora_weaponClass = myPtr;

		FActorSpawnParameters myP = FActorSpawnParameters();
		myP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		personalWeapon = Cast<AWeapon>(GetWorld()->SpawnActor<AWeapon>(myPtr, FVector::ZeroVector, FRotator(0, 0, 0), myP));
		personalWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "right_hand_socket");
		personalWeapon->UseWeaponAsActorOrientation(GetMesh()->GetSocketLocation("weapon_point_socket") - GetMesh()->GetSocketLocation("right_hand_socket"));
		personalWeapon->GetWeaponMesh()->SetHiddenInGame(true);
		personalWeapon->InitHeldMode(this);
		personalWeapon->canScan = false;
	}
}

void AServantAurora::HandleAttackMode()
{

	FVector dirToTarget = playerCharacter->GetActorLocation() - GetActorLocation();
	float distanceToTarget = dirToTarget.Size();

	if (!readyForNextAttack || attackWait > 0) return;

	if (distanceToTarget < desiredStandoff || isInOverrun)
	{
		if (doingcombo)
		{
			if (comboCounter < maxCombo)
			{
				specialAttackCode = FString::FromInt(currentComboList[comboCounter]);
				TryAttack();
			}
			comboCounter++;

			if (comboCounter >= maxCombo)
			{
				ResetCombo();
				nextResetInitWait = true;
			}
		}
		else
		{
			doingcombo = true;
			currentComboIndex = FMath::RandRange((int)0, (int)comboList.Num()-1);
			currentComboList = comboList[currentComboIndex];
			maxCombo = currentComboList.Num();
			specialAttackCode = FString::FromInt(currentComboList[0]);
			TryAttack();
			comboCounter++;
		}
	}

}

void AServantAurora::ResetCombo()
{
	currentComboIndex = 0;
	specialAttackCode = "0";
	comboCounter = 0;
	doingcombo = false;
}

void AServantAurora::DoAuroraIceSpawn()
{
	if (!aurora_weaponClass) return;
	FActorSpawnParameters myP = FActorSpawnParameters();
	myP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeapon* spawnedWeapon = Cast<AWeapon>(GetWorld()->SpawnActor<AWeapon>(aurora_weaponClass, GetMesh()->GetSocketLocation("Muzzle_02"), FRotator(0, 0, 0), myP));
	InitSpawnedWeapon(false, spawnedWeapon);
	spawnedWeapon->dumb = true;
	UGameplayStatics::SpawnEmitterAttached(spawnedIceParticles, spawnedWeapon->GetWeaponMesh(), FName(""), FVector::ZeroVector, FRotator::ZeroRotator);
	spawnedWeapon->InitProjectileMode(FVector::ZeroVector, this, playerCharacter, .25);
}

void AServantAurora::ReadyForNextAttack(bool r)
{
	readyForNextAttack = r;
	if (nextResetInitWait)
	{
		attackWait = FMath::FRandRange(2, 4);
		nextResetInitWait = false;
	}
}

#pragma endregion