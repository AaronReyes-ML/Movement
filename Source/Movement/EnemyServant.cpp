// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyServant.h"
#include "Components/CapsuleComponent.h"
#include "ServantAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Weapon.h"
#include "AIManager.h"
#include "MyTypes.h"

AEnemyServant::AEnemyServant()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> meshObject1(TEXT("SkeletalMesh'/Game/ParagonAurora/Characters/Heroes/Aurora/Meshes/Aurora.Aurora'"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> animInstance1(TEXT("/Game/EnemyServantAnim/Aurora/Servant_Aurora.Servant_Aurora_C"));

	GetMesh()->SetSkeletalMesh(meshObject1.Object);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetAnimInstanceClass(animInstance1.Class);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -97), FRotator(0, -90, 0));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECollisionResponse::ECR_Ignore);

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	isPlayer = false;
	invincible = true;
	doNothing = true;

	Tags.Add("Targetable");
}

void AEnemyServant::BeginPlay()
{
	Super::Super::BeginPlay();

	health = 10;
	armor = 1;

	GetCharacterMovement()->MaxWalkSpeed = (600);

	currentMode = EESM_Attack;
	timeToCycleMode = 20;
	animInstance = Cast<UServantAnimInstance>(GetMesh()->GetAnimInstance());

	InitWeaponForEnemy();
}

void AEnemyServant::SetRefPlayer(AServant* p)
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

void AEnemyServant::Tick(float DeltaTime)
{
	Super::Super::Tick(DeltaTime);

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

#pragma region Mode

void AEnemyServant::ForceEnemyMode(EESnemyMode forceMode)
{
	if (forceMode == EESM_Attack) timeToCycleMode = FMath::FRandRange(30, 45);
	else timeToCycleMode = FMath::FRandRange(1, 3);
	currentMode = forceMode;
}

void AEnemyServant::HandleModeCycle(float DeltaTime)
{
	timeToCycleMode -= DeltaTime;
	if (timeToCycleMode < 0)
	{
		CycleMode();
	}
}

void AEnemyServant::CycleMode()
{
	if (currentMode == EESM_Attack)
	{
		currentMode = EESM_Wait;
		timeToCycleMode = FMath::FRandRange(3, 5);
	}
	else
	{
		currentMode = EESM_Attack;
		timeToCycleMode = FMath::FRandRange(15, 20);
		rnd = FMath::RandBool() ? 1 : -1;
		comboCounter = 0;
		doingcombo = false;
		finishcombowithspecial = false;
	}
}

#pragma endregion

#pragma region Combat

void AEnemyServant::ReadyForNextAttack(bool r)
{
	readyForNextAttack = r;
}

void AEnemyServant::InitWeaponForEnemy()
{
	int weaponIndex = FMath::RandRange(0, 5);

	TSoftClassPtr<AWeapon> weaponClass = TSoftClassPtr<AWeapon>(FSoftObjectPath(TEXT("Blueprint'/Game/Weapons/Noble/MyNobleAurora.MyNobleAurora_C'")));;

	if (weaponClass)
	{
		UClass* myPtr = weaponClass.LoadSynchronous();

		FActorSpawnParameters myP = FActorSpawnParameters();
		myP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		personalWeapon = Cast<AWeapon>(GetWorld()->SpawnActor<AWeapon>(myPtr, FVector::ZeroVector, FRotator(0, 0, 0), myP));
		personalWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "right_hand_socket");
		personalWeapon->OrientSelfToWeapon(2);
		personalWeapon->GetWeaponMesh()->SetHiddenInGame(true);
		personalWeapon->InitHeldMode(this);
		personalWeapon->canScan = false;
	}
	else
	{
		IMyTypes::Debug_Print("Failed to init weapon", 10);
	}

}

void AEnemyServant::HandleAttackMode()
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
				specialAttackCode = "0";
				TryAttack();
			}
			else
			{
				if (finishcombowithspecial)
				{
					if (FMath::RandBool()) specialAttackCode = "Attack_Charge";
					else if (FMath::RandBool()) specialAttackCode = "Attack_Ring";
					else specialAttackCode = "Attack_Ult";
					TryAttack();
				}
			}
			comboCounter++;

			if (comboCounter > maxCombo)
			{
				ResetCombo();
				attackWait = FMath::FRandRange(2, 4);
			}
		}
		else
		{
			doingcombo = true;
			finishcombowithspecial = FMath::RandBool();
			maxCombo = FMath::RandRange(1, 4);
			specialAttackCode = "0";
			TryAttack();
			comboCounter++;
		}
	}

}

void AEnemyServant::ResetCombo()
{
	specialAttackCode = "0";
	comboCounter = 0;
	doingcombo = false;
	finishcombowithspecial = false;
}

void AEnemyServant::TryAttack()
{
	OrientDirectToTarget();
	animInstance->SetDesiredNextAttack(specialAttackCode);
	animInstance->AttackCommand(false, 1);
}

void AEnemyServant::DoDamage(float dmgIn)
{
	if (invincible || noHit) return;

	if (currentMode != EESM_Attack) ForceEnemyMode(EESM_Attack);

	if (dmgIn > 1) dmgIn = dmgIn - 1;
	if (health < 0 || dmgIn <= 0) return;

	health -= dmgIn;

	if (health < 0)
	{
		CharacterDestroyByDamage();
	}
}

void AEnemyServant::CharacterDestroyByDamage()
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


	if (personalWeapon)
	{
		personalWeapon->DoWeaponDropAndRemove();
	}
	if (aiManager) aiManager->ReportEnemyDestroyed(this);

	animInstance->WeaponDestroyed();
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(IMyTypes::Weapon, ECR_Ignore);
}

#pragma endregion

#pragma region movement

void AEnemyServant::OrientDirectToTarget()
{
	FVector dirToTarget = (playerCharacter->GetActorLocation() - GetActorLocation()) * FVector(1, 1, 0);
	dirToTarget.Normalize();
	SetActorRotation(dirToTarget.ToOrientationRotator());
}

void AEnemyServant::MoveToTarget()
{
	if (movementRestricted) return;

	if (currentMode == EESM_Attack)
	{
		FVector dirToTarget = (playerCharacter->GetActorLocation() - GetActorLocation()) * FVector(1,1,0);
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), dirToTarget.ToOrientationRotator(), GetWorld()->GetDeltaSeconds(), 3));

		float distToTarget = dirToTarget.Size();
		dirToTarget.Normalize();

		int multiplier = 600;

		if (isInOverrun)
		{
			if (distToTarget > softBoundRegionSize + desiredStandoff)
			{
				isInOverrun = false;
			}
			else
			{
				FVector finalDir = FVector::CrossProduct(dirToTarget, FVector::UpVector) * rnd;
				AddMovementInput(finalDir.GetSafeNormal() * 50, .1);
			}

			return;
		}
		else
		{
			if (distToTarget < desiredStandoff)
			{
				isInOverrun = true;
				softBoundRegionSize = FMath::RandRange(35, 85);
			}
		}


		AddMovementInput(dirToTarget * multiplier, 1);
	}
	else if (currentMode == EESM_Wait)
	{
		FVector dirToTarget = (playerCharacter->GetActorLocation() - GetActorLocation()) * FVector(1, 1, 0);
		FVector finalDir = FVector::CrossProduct(dirToTarget, FVector::UpVector) * rnd;

		SetActorRotation(FMath::RInterpTo(GetActorRotation(), dirToTarget.ToOrientationRotator(), GetWorld()->GetDeltaSeconds(), 3));

		AddMovementInput(finalDir * 300, 1);
	}

}

#pragma endregion