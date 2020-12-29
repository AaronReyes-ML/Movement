// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"
#include "Components/StaticMeshComponent.h"
#include "ConstructorHelpers.h"
#include "MyTypes.h"
#include "Servant.h"
#include "EnemyServant.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "TargetableComponent.h"
#include "Animation/BlendSpace1D.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveFloat.h"
#include "Particles/ParticleSystem.h"
#include "Engine.h"
#include "GameFramework/ForceFeedbackEffect.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> mesh0(TEXT("/Game/Weapon_Pack/Skeletal_Mesh/SK_Sword"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> asset1(TEXT("/Game/FX/Trace/TraceWeapon_Sword"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> asset2(TEXT("/Game/FX/Trace/Trace_Fail"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> asset3(TEXT("/Game/FX/Trace/TraceOther_Sword"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> asset4(TEXT("/Game/FX/Trace/Trace_Success"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> asset5(TEXT("NiagaraSystem'/Game/FX/WeaponDestroy/WeaponDestroy.WeaponDestroy'"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> asset6(TEXT("NiagaraSystem'/Game/FX/WeaponDestroy/Weapon_Shed.Weapon_Shed'"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> asset7(TEXT("ParticleSystem'/Game/ParagonAurora/FX/Particles/Abilities/Ultimate/FX/P_Aurora_Ultimate_Explode.P_Aurora_Ultimate_Explode'"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> asset8(TEXT("ParticleSystem'/Game/ParagonGreystone/FX/Particles/Greystone/Abilities/Primary/FX/P_Greystone_Primary_Impact.P_Greystone_Primary_Impact'"));

	static ConstructorHelpers::FObjectFinder<UAnimMontage> attackMontage1(TEXT("AnimMontage'/Game/ServantAnim/AnimationsByWeapon/Sword/Combat/Sword_Combat_Montage.Sword_Combat_Montage'"));
	static ConstructorHelpers::FObjectFinder<UBlendSpace1D> animAsset1(TEXT("BlendSpace1D'/Game/ServantAnim/AnimationsByWeapon/Sword/Movement/Sword_Movement_BS.Sword_Movement_BS'"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> animAsset2(TEXT("/Game/ServantAnim/AnimationsByWeapon/Sword/Movement/Sword_Idle"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> animAsset3(TEXT("/Game/ServantAnim/AnimationsByWeapon/Sword/Combat/Sword_TraceON"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> animAsset4(TEXT("AnimMontage'/Game/Damage_Montage.Damage_Montage'"));
	static ConstructorHelpers::FObjectFinder<UCurveVector> curveAsset1(TEXT("/Game/ServantAnim/AnimationsByWeapon/NewSword/Curves/Special1_Curve"));
	static ConstructorHelpers::FObjectFinder<UCurveVector> curveAsset2(TEXT("CurveVector'/Game/ServantAnim/AnimationsByWeapon/NewSword/Curves/AttackCurve1.AttackCurve1'"));
	static ConstructorHelpers::FObjectFinder<UCurveVector> curveAsset3(TEXT("CurveVector'/Game/ServantAnim/AnimationsByWeapon/NewSword/Curves/AttackCurve2.AttackCurve2'"));
	static ConstructorHelpers::FObjectFinder<UCurveVector> curveAsset4(TEXT("CurveVector'/Game/ServantAnim/AnimationsByWeapon/NewSword/Curves/AttackCurve3.AttackCurve3'"));
	static ConstructorHelpers::FObjectFinder<UCurveVector> curveAsset5(TEXT("CurveVector'/Game/ServantAnim/AnimationsByWeapon/NewSword/Curves/AttackCurve4.AttackCurve4'"));
	static ConstructorHelpers::FObjectFinder<UCurveVector> curveAsset6(TEXT("CurveVector'/Game/ServantAnim/AnimationsByWeapon/NewSword/Curves/AttackCurve5.AttackCurve5'"));
	static ConstructorHelpers::FObjectFinder<UCurveVector> curveAsset7(TEXT("CurveVector'/Game/ServantAnim/AnimationsByWeapon/NewSword/Curves/AttackCurve6.AttackCurve6'"));
	static ConstructorHelpers::FObjectFinder<UCurveFloat> curveAsset8(TEXT("CurveFloat'/Game/Weapons/Standard_DamageCurve.Standard_DamageCurve'"));
	static ConstructorHelpers::FObjectFinder<UForceFeedbackEffect> ffb1(TEXT("ForceFeedbackEffect'/Game/ServantAnim/AnimationsByWeapon/NewSword/Curves/AttackFFB.AttackFFB'"));

	

	if (!weaponSourceMesh) weaponSourceMesh = mesh0.Object;
	if (!weaponAttackMontage) weaponAttackMontage = attackMontage1.Object;
	if (!weaponFeedback) weaponFeedback = ffb1.Object;

	formSystem = asset1.Object;
	failSystem = asset2.Object;
	traceSelfSystem = asset3.Object;
	successSystem = asset4.Object;
	explodeSystem = asset5.Object;
	shedSystem = asset6.Object;
	explodeSystemNrmParticle = asset7.Object;
	weaponImpactParticle = asset8.Object;
	//movementAnim = animAsset1.Object;
	idleAnim = animAsset2.Object;
	traceOnMontage = animAsset3.Object;

	testCurve = curveAsset1.Object;
	attack1MovementCurve = curveAsset2.Object;
	attack2MovementCurve = curveAsset3.Object;
	attack3MovementCurve = curveAsset4.Object;
	attack4MovementCurve = curveAsset5.Object;
	attack5MovementCurve = curveAsset6.Object;
	attack6MovementCurve = curveAsset7.Object;
	attackDamageCurve = curveAsset8.Object;

	damageMontageTest = animAsset4.Object;

	weaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Mesh");
	SetRootComponent(weaponMesh);
	weaponMesh->SetHiddenInGame(false);
	weaponMesh->SetSkeletalMesh (weaponSourceMesh);
	weaponMesh->SetCollisionObjectType(IMyTypes::Weapon);
	weaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	weaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	weaponMesh->SetCollisionResponseToChannel(IMyTypes::Weapon, ECR_Overlap);
	weaponMesh->SetupAttachment(RootComponent);
	
	searchOverlap = CreateDefaultSubobject<UBoxComponent>("Search overlap");
	searchOverlap->SetupAttachment(RootComponent);
	searchOverlap->SetCollisionObjectType(IMyTypes::Weapon_Srch);
	searchOverlap->SetCollisionResponseToAllChannels(ECR_Overlap);

	weaponSize.Z = ((weaponMesh->GetSocketLocation(FName("end")) - weaponMesh->GetSocketLocation(FName("start"))).Size()) / 2;
	weaponSize.Y = 25;
	weaponSize.X = 25;
	searchOverlap->SetBoxExtent(weaponSize);
	searchOverlap->SetRelativeLocation(FVector(0, 0, (-weaponSize.Z) + weaponMesh->GetSocketTransform(FName("start"), ERelativeTransformSpace::RTS_Actor).GetLocation().X));
}

AWeapon::AWeapon(FVector initSpawnVector, FRotator initSpawnDir)
{

}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	weaponMesh->SetHiddenInGame(hideWeaponMesh);

	weaponMat = weaponMesh->CreateDynamicMaterialInstance(0);

	actorToWeaponDeltaRotator = GetActorRotation() - GetWeaponDir().ToOrientationRotator();

	weaponMesh->OnComponentHit.AddDynamic(this, &AWeapon::OnWeaponCollision);
	weaponMesh->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnOverlapBeginOther);

	if (forceCanScan) canScan = true;
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (debugFaceTarget && debugTarget)
	{
		FVector dirToDebugTarget = debugTarget->GetActorLocation() - GetActorLocation();
		UseWeaponAsActorOrientation(dirToDebugTarget);
	}
	

	if (weaponDanger)
	{
		FVector vel = prevFramePointLoc - weaponMesh->GetSocketLocation("end");
		weaponDamageVelocity = vel.Size();
	}

	if (weaponOwner)
	{
		if (weaponOwner->GetIsPlayer())
		{
			//IMyTypes::Debug_Print(currentAddScanTime, 1, 1);
		}
	}

	switch (currentWeaponMode)
	{
	case AWeapon::EWM_None:
		if (inSpawnSequence) TimeoutSpawnSequence(DeltaTime);
		if (jitterSelf) JitterSelfLoc(DeltaTime);
		if (isScanning && accumulateScanTime > 0) AddScanTime(DeltaTime);
		else if (isScanning && lastScanMiss) AddMissTime(DeltaTime);
		break;
	case AWeapon::EWM_Held:
		if (inSpawnSequence) TimeoutSpawnSequence(DeltaTime);
		else if (isProcessingAttack) ProcessAttack(DeltaTime);
		if (isScanning && accumulateScanTime > 0) AddScanTime(DeltaTime);
		else if (isScanning && lastScanMiss) AddMissTime(DeltaTime);
		break;
	case AWeapon::EWM_Projectile:
		if (inSpawnSequence) TimeoutSpawnSequence(DeltaTime);
		else if (!initProjectileDone) DoInitProjectileSequence(DeltaTime);
		else ProjectileSeekTarget();
		break;
	case AWeapon::EWM_Disabled:
		break;
	default:
		break;
	}

	if (isCountingDown) HandleSelfDestroy(DeltaTime);


}

#pragma region combat

void AWeapon::InitHeldMode(AServant* other)
{
	weaponOwner = other;
	currentWeaponMode = EWeaponMode::EWM_Held;
}

void AWeapon::StartAttackHandler(int inAttackCode, float inComboDepth, float addScanTime)
{
	if (!weaponOwner) return;
	currentAttackSpecialEffect = "None";

	currentAttackCode = inAttackCode;
	currentComboDepth = FMath::Abs(inComboDepth);

	if (addScanTime != 0) currentAddScanTime += addScanTime;
	else
	{
		currentAddScanTime -= .25;
		if (currentAddScanTime < 0) currentAddScanTime = 0;
	}
	

	isProcessingAttack = true;
	processThisAttack = true;

	timeInAttack = 0;

	if (attackDataArray.Num() >= currentAttackCode && attackDataArray.Num() > 0)
	{
		currentAttackData = attackDataArray[currentAttackCode - 1];
		processThisAttack = currentAttackData.processThis;
		currentAttackSpecialEffect = currentAttackData.hitEffect;
		//if (currentAttackData.forceMaxMovement) weaponOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_MAX);
		//if (currentAttackData.canIgnorePawn) weaponOwner->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}

}

void AWeapon::EndAttackHandler(int inAttackCode)
{
	if (currentAttackCode != inAttackCode) return;

	if (IsValid(attackOverlapVolume))
	{
		if (!attackOverlapVolume->IsBeingDestroyed())
		{
			attackOverlapVolume->DestroyComponent();
			attackOverlapVolume = nullptr;
		}
	}

	isProcessingAttack = false;
	timeInAttack = 0;

}

void AWeapon::StartWeaponDangerous()
{
	if (!weaponOwner) return;

	weaponDanger = true;
	weaponDamageVelocity = 0;
	prevFramePointLoc = weaponMesh->GetSocketLocation("end");

	weaponOwner->EmptyAttackAffectedActors();
	weaponMesh->SetGenerateOverlapEvents(true);
	weaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWeapon::EndWeaponDangerous()
{
	if (!weaponOwner) return;

	weaponDanger = false;
	weaponMesh->SetGenerateOverlapEvents(false);
	weaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::ProcessAttack(float DeltaTime)
{
	/*
	if (!processThisAttack) return;
	timeInAttack += DeltaTime;
	if (!weaponOwner) return;
	if (currentAttackCode == 1) ProcessAttack1(DeltaTime);
	if (currentAttackCode == 2) ProcessAttack2(DeltaTime);
	if (currentAttackCode == 3) ProcessAttack3(DeltaTime);
	if (currentAttackCode == 4) ProcessAttack4(DeltaTime);
	if (currentAttackCode == 5) ProcessAttack5(DeltaTime);
	if (currentAttackCode == 6) ProcessAttack6(DeltaTime);
	*/
}

void AWeapon::ProcessAttack1(float DeltaTime)
{
	if (!weaponOwner) return;

	if (currentAttackData.canControlMove && attack1MovementCurve)
	{
		weaponOwner->AddActorWorldOffset((attack1MovementCurve->GetVectorValue(timeInAttack).X * weaponOwner->GetActorForwardVector() * DeltaTime) +
			(attack1MovementCurve->GetVectorValue(timeInAttack).Y * weaponOwner->GetActorRightVector() * DeltaTime) +
			(attack1MovementCurve->GetVectorValue(timeInAttack).Z * weaponOwner->GetActorUpVector() * DeltaTime)
			, true, nullptr, ETeleportType::TeleportPhysics);
	}

}

void AWeapon::ProcessAttack2(float DeltaTime)
{
	if (!weaponOwner) return;

	if (currentAttackData.canControlMove && attack2MovementCurve)
	{
		weaponOwner->AddActorWorldOffset((attack2MovementCurve->GetVectorValue(timeInAttack).X * weaponOwner->GetActorForwardVector() * DeltaTime) +
			(attack2MovementCurve->GetVectorValue(timeInAttack).Y * weaponOwner->GetActorRightVector() * DeltaTime) +
			(attack2MovementCurve->GetVectorValue(timeInAttack).Z * weaponOwner->GetActorUpVector() * DeltaTime)
			, true, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ProcessAttack3(float DeltaTime)
{
	if (!weaponOwner) return;

	if (currentAttackData.canControlMove && attack3MovementCurve)
	{
		weaponOwner->AddActorWorldOffset((attack3MovementCurve->GetVectorValue(timeInAttack).X * weaponOwner->GetActorForwardVector() * DeltaTime) +
			(attack3MovementCurve->GetVectorValue(timeInAttack).Y * weaponOwner->GetActorRightVector() * DeltaTime) +
			(attack3MovementCurve->GetVectorValue(timeInAttack).Z * weaponOwner->GetActorUpVector() * DeltaTime)
			, true, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ProcessAttack4(float DeltaTime)
{
	if (!weaponOwner) return;

	if (currentAttackData.canControlMove && attack4MovementCurve)
	{
		weaponOwner->AddActorWorldOffset((attack4MovementCurve->GetVectorValue(timeInAttack).X * weaponOwner->GetActorForwardVector() * DeltaTime) +
			(attack4MovementCurve->GetVectorValue(timeInAttack).Y * weaponOwner->GetActorRightVector() * DeltaTime) +
			(attack4MovementCurve->GetVectorValue(timeInAttack).Z * weaponOwner->GetActorUpVector() * DeltaTime)
			, true, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ProcessAttack5(float DeltaTime)
{
	if (!weaponOwner) return;

	if (currentAttackData.canControlMove && attack5MovementCurve)
	{
		weaponOwner->AddActorWorldOffset((attack5MovementCurve->GetVectorValue(timeInAttack).X * weaponOwner->GetActorForwardVector() * DeltaTime) +
			(attack5MovementCurve->GetVectorValue(timeInAttack).Y * weaponOwner->GetActorRightVector() * DeltaTime) +
			(attack5MovementCurve->GetVectorValue(timeInAttack).Z * weaponOwner->GetActorUpVector() * DeltaTime)
			, true, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ProcessAttack6(float DeltaTime)
{
	if (!weaponOwner) return;

	if (currentAttackData.canControlMove && attack5MovementCurve)
	{
		weaponOwner->AddActorWorldOffset((attack6MovementCurve->GetVectorValue(timeInAttack).X * weaponOwner->GetActorForwardVector() * DeltaTime) +
			(attack6MovementCurve->GetVectorValue(timeInAttack).Y * weaponOwner->GetActorRightVector() * DeltaTime) +
			(attack6MovementCurve->GetVectorValue(timeInAttack).Z * weaponOwner->GetActorUpVector() * DeltaTime)
			, true, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ProcessEnemyHitEvent(AActor* otherActor)
{
	AServant* otherServ = Cast<AServant>(otherActor);
	if (!otherServ) return;

	float damage = weaponDamage * (attackDamageCurve ? attackDamageCurve->GetFloatValue(currentComboDepth) : 1);

	weaponOwner->DoHitOther(otherActor);
	weaponOwner->DoPlayFFB(weaponFeedback);
	otherServ->DoTakeHit(currentAttackSpecialEffect);
	otherServ->DoDamage(damage * FMath::RandRange(1.f, weaponDamage_MaxDeviance));

	if (!otherServ->GetIsInvincible()) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), weaponImpactParticle, GetPositionBetweenWeapon());
	
	if (weaponDestroyable) ApplyWeaponDurabilityDamage();

	if (weaponOwner->GetIsPlayer())
	{
		weaponOwner->AddSpecPercent(.025);
		weaponOwner->AddCrestPercent(.01);
	}
	
	if (!otherServ->GetIsPlayer() && currentAddScanTime > 0)
	{
		AWeapon* toTrace = otherServ->GetPersonalWeapon();
		if (!toTrace) return;
		if (!toTrace->canScan) return;

		weaponOwner->TryTraceWeapon(toTrace);
		toTrace->accumulateScanTime += currentAddScanTime;
		toTrace->lastScanMiss = true;
	}

}

void AWeapon::ApplyWeaponDurabilityDamage()
{
	if (!takeDurabilityDamage) return;
	weaponDurability -= (weaponDRH * FMath::FRandRange(1, weaponDRH_MaxDeviance));

	if (weaponMat && weaponMesh)
	{
		weaponMat->SetScalarParameterValue("Cracks_Depth", wipeStart + ((1-weaponDurability) * (wipeEnd - wipeStart)));
		weaponMesh->SetMaterial(0, weaponMat);
	}

	if (weaponDurability < 0)
	{
		if (!nobleWeapon)
		{
			WeaponDestroyByDurability();
		}
		else
		{
			weaponOwner->ReportNobleWeaponUseOver();
		}

	}
}

void AWeapon::WeaponDestroyByDurability()
{
	if (weaponOwner)
	{
		weaponOwner->ReportDestroyedWeapon();
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		DoWeaponDestroyAndRemove();
	}
}

void AWeapon::DoAOE()
{
	if (weaponOwner)
	{
		if (!weaponOwner->GetIsPlayer()) return;
		if (nobleWeapon) weaponOwner->DoAOEImmediate(1500, weaponMesh->GetComponentLocation(), false, 10.25, weaponDamage);
		else weaponOwner->DoAOEImmediate(500, weaponMesh->GetComponentLocation(), false, .125, weaponDamage);
	}
}

#pragma endregion

#pragma region overlap and collision

void AWeapon::OnWeaponCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult &Hit)
{
	if (currentWeaponMode == EWeaponMode::EWM_Projectile)
	{
		if (OtherActor->IsA(AWeapon::StaticClass()))
		{
			if (Cast<AWeapon>(OtherActor)->self) return;
		}

		DoWeaponDestroyAndRemove();
	}
}

void AWeapon::OnOverlapBeginOther(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == weaponOwner) return;

	if (currentWeaponMode == EWeaponMode::EWM_Projectile)
	{
		if (OtherActor->IsA(AServant::StaticClass()))
		{
			if (!weaponOwner->IsActorAffectedByCurrentAttack(OtherActor))
			{
				AServant* otherServ = Cast<AServant>(OtherActor);
				if (otherServ->GetIsInvincible()) return;
				if (nobleWeapon)
				{
					if (otherServ->GetIsPlayer()) otherServ->DoDamage(weaponDamage);
					else otherServ->DoDamage(5 * weaponDamage);
				}
				else otherServ->DoDamage(weaponDamage);

				otherServ->DoTakeHit("KnockBack");
			}
			DoWeaponDestroyAndRemove();
		}
	}
	else if (currentWeaponMode == EWeaponMode::EWM_Held)
	{
		if (!weaponOwner) return;
		if (weaponOwner->IsA(AEnemyServant::StaticClass()) && OtherActor->IsA(AEnemyServant::StaticClass())) return;

		if (OtherActor->IsA(AServant::StaticClass()))
		{
			if (!weaponOwner->IsActorAffectedByCurrentAttack(OtherActor))
			{
				ProcessEnemyHitEvent(OtherActor);
			}
		}
	}

}

void AWeapon::OnOverlapEndOther(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (otherActor == weaponOwner) return;

	if (currentWeaponMode == EWeaponMode::EWM_Held)
	{

	}
}

#pragma endregion

#pragma region removal

void AWeapon::DoWeaponDestroyAndRemove()
{
	DoAOE();
	noUpdateCollision = true;
	weaponMesh->SetHiddenInGame(true);
	weaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	selfDestroyCountdown = 2;
	UNiagaraFunctionLibrary::SpawnSystemAttached(explodeSystem, weaponMesh, FName("None"), FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTargetIncludingScale, true, true);
	if (currentWeaponMode == EWM_Projectile)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), explodeSystemNrmParticle, weaponMesh->GetComponentLocation());
	}
	currentWeaponMode = EWeaponMode::EWM_Disabled;

	InitSelfDestroySequence();
}

void AWeapon::DoWeaponDropAndRemove()
{
	weaponOwner = nullptr;
	noUpdateCollision = true;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (nobleWeapon)
	{
		weaponMesh->SetHiddenInGame(false);
		selfDestroyCountdown = 600;
		canScan = true;
		timeToFullScan = 3;
	}
	else
	{
		weaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		weaponMesh->SetSimulatePhysics(true);
	}
	InitSelfDestroySequence();
}

void AWeapon::InitSelfDestroySequence()
{
	isCountingDown = true;
}

void AWeapon::HandleSelfDestroy(float DeltaTime)
{
	selfDestroyCountdown -= DeltaTime;

	if (selfDestroyCountdown < 0)
	{
		if (scanningActor)
		{
			scanningActor->ReportTraceFailed(this);
		}
		Destroy();
	}
}

#pragma endregion

#pragma region spawning

void AWeapon::KickOffSpawnSequence()
{
	weaponDestroyable = true;
	inSpawnSequence = true;
	spawnSequenceTime = spawnSequenceTimeMax;

	formSelfComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(formSystem, weaponMesh, FName("None"), FVector::ZeroVector, FRotator::ZeroRotator,
		EAttachLocation::SnapToTargetIncludingScale, true);
	formSelfComponent->SetNiagaraVariableFloat("lifetime", spawnSequenceTimeMax);
	formSelfComponent->Activate();

}

void AWeapon::TimeoutSpawnSequence(float DeltaTime)
{
	spawnSequenceTime -= DeltaTime;

	if (spawnSequenceTime < 0)
	{
		inSpawnSequence = false;
		weaponMesh->SetHiddenInGame(false);
		hideWeaponMesh = false;
	}
}

#pragma endregion

#pragma region projectile

void AWeapon::InitProjectileMode(FVector inTangent, AServant* owner, AActor* trgt, float addInitTime)
{
	self = true;
	weaponOwner = owner;
	currentWeaponMode = EWeaponMode::EWM_Projectile;
	weaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	weaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	weaponMesh->SetSimulatePhysics(true);
	weaponMesh->SetEnableGravity(false);
	inTangent.Normalize();
	projectileTarget = trgt;
	initProjectileTime += addInitTime;

	initQuat = GetActorQuat();
	SetActorRotation(GetDirectionToTarget().ToOrientationQuat(), ETeleportType::TeleportPhysics);
	AddActorLocalRotation(actorToWeaponDeltaRotator.Quaternion(), false, nullptr, ETeleportType::TeleportPhysics);
	finalQuat = GetActorQuat();
	SetActorRotation(initQuat, ETeleportType::TeleportPhysics);

	UNiagaraFunctionLibrary::SpawnSystemAttached(shedSystem, weaponMesh, FName("None"), FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTargetIncludingScale, true, true);


	if (inTangent != FVector::ZeroVector)
		weaponMesh->SetPhysicsLinearVelocity(inTangent * 200);

}

void AWeapon::InitProjectileImmediate(AServant* owner, AActor* trgt)
{
	self = true;
	weaponOwner = owner;

	projectileTarget = trgt;

	//SetActorRotation(GetDirectionToTarget().ToOrientationQuat(), ETeleportType::TeleportPhysics);
	//OrientSelfToWeapon(4);

	UseWeaponAsActorOrientation(GetDirectionToTarget());

	currentWeaponMode = EWeaponMode::EWM_Projectile;
	weaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	weaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	weaponMesh->SetSimulatePhysics(true);
	weaponMesh->SetEnableGravity(false);

	projectileSeekWaitTime = .1;

	initProjectileDone = true;
	SetJitterSelf(false);
	SetProjectileCollision();
	InitSelfDestroySequence();

	weaponMesh->BodyInstance.SetInstanceNotifyRBCollision(true);
	weaponMesh->SetNotifyRigidBodyCollision(true);
	weaponMesh->SetGenerateOverlapEvents(true);

	weaponMesh->SetPhysicsLinearVelocity(directionToTarget * 6000);
	UseWeaponAsActorOrientation(directionToTarget);
	InitSelfDestroySequence();
}

void AWeapon::SetProjectileCollision()
{
	weaponMesh->BodyInstance.SetInstanceNotifyRBCollision(true);
	weaponMesh->SetNotifyRigidBodyCollision(true);
	weaponMesh->SetGenerateOverlapEvents(true);
}

void AWeapon::DoInitProjectileSequence(float DeltaTime)
{
	initProjectileTime -= DeltaTime;

	initQuat = GetActorQuat();
	SetActorRotation(GetDirectionToTarget().ToOrientationQuat(), ETeleportType::TeleportPhysics);
	AddActorLocalRotation(actorToWeaponDeltaRotator.Quaternion(), false, nullptr, ETeleportType::TeleportPhysics);
	finalQuat = GetActorQuat();
	SetActorRotation(initQuat, ETeleportType::TeleportPhysics);

	SetActorRotation(FMath::QInterpTo(GetActorQuat(), finalQuat, DeltaTime, 4), ETeleportType::TeleportPhysics);

	if (initProjectileTime <= 0)
	{
		SetProjectileCollision();
		InitSelfDestroySequence();
		initProjectileDone = true;
		weaponMesh->SetPhysicsLinearVelocity(directionToTarget * 6000);
		UseWeaponAsActorOrientation(directionToTarget);
	}
}

void AWeapon::IdentifyProjectileTarget()
{
	TArray<AActor*> foundActors;

	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AServant::StaticClass(), FName("Targetable"), foundActors);
	float minDist = FLT_MAX;

	AServant* retActor = nullptr;

	for (int i = 0; i < foundActors.Num(); i++)
	{
		AServant* foundServ = Cast<AServant>(foundActors[i]);
		if (!foundServ) continue;

		if (foundServ == projectileTarget || foundServ->GetHealth() < 0 || foundServ == weaponOwner) continue;
		FVector dirTo = foundServ->GetActorLocation() - GetActorLocation();
		if (dirTo.Size() < minDist)
		{
			minDist = dirTo.Size();
			retActor = foundServ;
		}
	}

	projectileTarget = retActor;
}

FVector AWeapon::GetDirectionToTarget()
{
	if (projectileTarget)
	{
		//IMyTypes::Debug_Print("Target: " + projectileTarget->GetName(), 3);
		if (projectileTarget->IsA(AServant::StaticClass()))
		{
			if (Cast<AServant>(projectileTarget)->GetHealth() < 0)
			{
				IdentifyProjectileTarget();
				return GetDirectionToTarget();
			}
		}
		directionToTarget = projectileTarget->GetActorLocation() - GetActorLocation();
	}
	else
	{
		//IMyTypes::Debug_Print("NO TARGET: ", 3);
		directionToTarget = FVector::ZeroVector - GetActorLocation();
	}

	directionToTarget.Normalize();

	return directionToTarget;
}

void AWeapon::ProjectileSeekTarget()
{
	if (dumb) return;
	FVector currentVel = weaponMesh->GetPhysicsLinearVelocity();
	currentVel.Normalize();

	//SetActorRotation(GetDirectionToTarget().ToOrientationQuat(), ETeleportType::TeleportPhysics);
	//OrientSelfToWeapon(4);

	UseWeaponAsActorOrientation(GetDirectionToTarget());

	weaponMesh->SetPhysicsLinearVelocity(FMath::VInterpTo(currentVel, directionToTarget, GetWorld()->GetDeltaSeconds(), 5)  * 6000);

}

#pragma endregion

#pragma region trace

void AWeapon::SelfTraceBegin()
{
	if (!canScan && !infiniteScan) return;

	InitTraceParticles();

	isScanning = true;
}

void AWeapon::SelfTraceComplete()
{
	if (!scanningActor) return;

	InitCompleteParticles(true);

	scanningActor->ReportFullTrace(this);

	ResetScan();
	if (!infiniteScan)
	{
		canScan = false;
	}
}

void AWeapon::SelfTraceFail()
{
	if (!scanningActor) return;

	InitCompleteParticles(false);

	scanningActor->ReportTraceFailed(this);
	ResetScan();
}

void AWeapon::InitTraceParticles()
{
	if (weaponMat && weaponMesh)
	{
		weaponMat->SetScalarParameterValue("Wipe_Depth", wipeStart);
		weaponMesh->SetMaterial(0, weaponMat);
	}
}

void AWeapon::InitCompleteParticles(bool success)
{
	if (!scanningActor) return;

	if (weaponMat && weaponMesh)
	{
		weaponMat->SetScalarParameterValue("Wipe_Depth", -999);
		weaponMesh->SetMaterial(0, weaponMat);
	}


	if (success)
	{
		traceFinishComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(successSystem, scanningActor->GetMesh(), FName("spine_01"), FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget, true);
		traceFinishComponent->Activate();
	}
	else
	{
		traceFinishComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(failSystem, weaponMesh, FName("start"), FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget, true);
		traceFinishComponent->Activate();
	}

}

void AWeapon::SetScanLocation()
{


}

void AWeapon::AddScanTime(float DeltaSeconds)
{
	if (!isScanning)
	{
		SelfTraceBegin();
	}

	if (accumulateScanTime > 0) accumulateScanTime -= DeltaSeconds;

	scanPercent += DeltaSeconds / timeToFullScan;
	validMissTime = validMissTimeMax;

	if (weaponMat && weaponMesh)
	{
		weaponMat->SetScalarParameterValue("Wipe_Depth", wipeStart + (scanPercent * (wipeEnd - wipeStart)));
		weaponMesh->SetMaterial(0, weaponMat);
	}

	if (scanPercent > 1.f)
	{
		SelfTraceComplete();
	}
}

void AWeapon::AddMissTime(float DeltaSeconds)
{
	if (accumulateScanTime > 0) return;
	validMissTime -= DeltaSeconds;

	//CustomTimeDilation = 1;
	if (validMissTime < 0)
	{
		SelfTraceFail();
	}
}

void AWeapon::ResetScan()
{
	isScanning = false;
	scanPercent = 0.f;
	validMissTime = validMissTimeMax;
}

void AWeapon::StartPreviewParticle()
{
	traceSelfComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(traceSelfSystem, weaponMesh, FName("none"), FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTargetIncludingScale,
		true, true);
}

void AWeapon::DestroyPreviewParticle()
{
	if (traceSelfComponent)
	{
		//traceSelfComponent->DeactivateImmediate();
	}
}

#pragma endregion

#pragma region utility
void AWeapon::UseWeaponAsActorOrientation(const FVector &setDir)
{
	SetActorRotation(setDir.ToOrientationQuat(), ETeleportType::TeleportPhysics);

	FVector weaponPoint = weaponMesh->GetSocketLocation("end") - weaponMesh->GetSocketLocation("start");
	FVector actorForward = GetActorForwardVector();

	FQuat offset = FQuat::FindBetween(weaponPoint, actorForward);
	SetActorRotation(offset, ETeleportType::TeleportPhysics);
}

void AWeapon::OrientSelfToWeapon(float mult)
{
	if (mult == 4)
	{
		float delta = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), GetWeaponDir()));
		AddActorLocalRotation(FRotator(selfMult3 * FMath::RadiansToDegrees(delta), 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
		return;
	}

	SetActorRelativeRotation(FRotator(0));
	if (mult == 2)
	{
		float delta = FMath::Acos(FVector::DotProduct(GetActorUpVector(), GetWeaponDir()));
		AddActorLocalRotation(FRotator(FMath::RadiansToDegrees(delta), selfMult2 * 90, 0), false, nullptr, ETeleportType::TeleportPhysics);
	}
	else if (mult == 3)
	{
		float delta = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), GetWeaponDir()));
		AddActorLocalRotation(FRotator(selfMult * FMath::RadiansToDegrees(delta) + 10,0, 0), false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		float delta = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), GetWeaponDir()));
		AddActorLocalRotation(FRotator(selfMult * FMath::RadiansToDegrees(delta), selfMult2 * 90, 0), false, nullptr, ETeleportType::TeleportPhysics);
	}

}

FVector AWeapon::GetWeaponDir()
{
	FVector retVec = weaponMesh->GetSocketLocation("end") - weaponMesh->GetSocketLocation("start");
	retVec.Normalize();
	return retVec;
}

FVector AWeapon::GetPositionBetweenWeapon()
{
	FVector retVec = weaponMesh->GetSocketLocation("end") - weaponMesh->GetSocketLocation("start");
	float len = retVec.Size();
	retVec.Normalize();

	float randDist = FMath::RandRange(0.f, len);

	return (weaponMesh->GetSocketLocation("start") + ((retVec) * randDist));
}

void AWeapon::SetMeshHidden(bool hide)
{
	weaponMesh->SetHiddenInGame(hide);
}

void AWeapon::JitterSelfLoc(float DeltaTime)
{
	totalTime = FMath::Clamp(totalTime + DeltaTime, 0.f, 10000.f);
	if (totalTime > 10000) totalTime = 0;
	float jitterAmount = FMath::Sin(totalTime) * 10 * DeltaTime;

	AddActorWorldOffset(jitterAmount * FVector::UpVector);
}

#pragma endregion
