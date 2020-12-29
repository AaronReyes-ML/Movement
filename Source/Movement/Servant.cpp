// Fill out your copyright notice in the Description page of Project Settings.

#include "Servant.h"
#include "ServantAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "ConstructorHelpers.h"
#include "Weapon.h"
#include "MyTypes.h"
#include "ServantUI.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine.h"
#include "Curves/CurveVector.h"
#include "AIManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/ForceFeedbackEffect.h"
#include "EnemyServant.h"
#include "ServantGameInstance.h"

#pragma region construct

// Sets default values
AServant::AServant()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> meshObject1(TEXT("/Game/Mannequin/Character/Mesh/SK_Mannequin"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> animInstance1(TEXT("/Game/ServantAnim/AnimBP/Mannequin_ServantAnimBP"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> animInstance2(TEXT("/Game/ServantAnim/AnimationsByWeapon/Bow/Combat/NobleBow_AnimBP"));
	static ConstructorHelpers::FClassFinder<UUserWidget> UI1(TEXT("/Game/UI/ServantUI_Bp"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> asset1(TEXT("/Game/FX/AimBeam"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> asset2(TEXT("NiagaraSystem'/Game/FX/Trace/Current.Current'"));
	static ConstructorHelpers::FObjectFinder<UCurveVector> curve1(TEXT("CurveVector'/Game/ServantAnim/AnimationsByWeapon/Movement/DodgeCurve.DodgeCurve'"));

	dodgeCurve = curve1.Object;

	breakBowAnimClass = animInstance2.Class;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 400.f, 0);

	GetCapsuleComponent()->SetCollisionResponseToChannel(IMyTypes::Weapon, ECR_Overlap);

	cameraBoom = CreateDefaultSubobject<USpringArmComponent>("Camera Boom");
	cameraBoom->SetupAttachment(RootComponent);
	cameraBoom->SetRelativeRotation(FRotator(-20, 0, 0));
	cameraBoom->SetRelativeLocation(FVector::ZeroVector);
	cameraBoom->TargetArmLength = 400;
	cameraBoom->bDoCollisionTest = true;
	cameraBoom->bUsePawnControlRotation = true;
	cameraBoom->bInheritPitch = true;
	cameraBoom->bInheritYaw = true;
	cameraBoom->bInheritRoll = true;
	cameraBoom->bDoCollisionTest = true;
	cameraBoom->bEnableCameraLag = true;
	cameraBoom->bEnableCameraRotationLag = true;

	targetingCameraBoom = CreateDefaultSubobject<USpringArmComponent>("Target Camera Boom");
	targetingCameraBoom->SetupAttachment(RootComponent);
	targetingCameraBoom->SetRelativeRotation(FRotator::ZeroRotator);
	targetingCameraBoom->SetRelativeLocation(FVector(0, 75, 0));
	targetingCameraBoom->TargetArmLength = 300;
	targetingCameraBoom->bDoCollisionTest = false;

	camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	camera->SetupAttachment(cameraBoom, cameraBoom->SocketName);
	camera->SetRelativeLocation(FVector::ZeroVector);
	camera->SetRelativeRotation(FRotator::ZeroRotator);
	camera->bUsePawnControlRotation = false;

	GetMesh()->SetSkeletalMesh(meshObject1.Object);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetAnimInstanceClass(animInstance1.Class);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -97), FRotator(0, -90, 0));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	traceBeam = CreateDefaultSubobject<UNiagaraComponent>("Trace beam");
	traceBeam->SetupAttachment(RootComponent);
	traceBeam->SetAsset(asset1.Object);
	traceBeam->bAutoActivate = false;

	availableWeaponsSystem = CreateDefaultSubobject<UNiagaraComponent>("Traced weapons particles");
	availableWeaponsSystem->SetupAttachment(RootComponent);
	availableWeaponsSystem->SetAsset(asset2.Object);
	availableWeaponsSystem->bAutoActivate = false;
	availableWeaponsSystem->SetNiagaraVariableInt("Count", 0);

	traceVolume = CreateDefaultSubobject<UBoxComponent>("Multi weapon scan box");
	traceVolume->SetupAttachment(RootComponent);
	traceVolume->SetRelativeLocation(FVector(0));
	traceVolume->SetBoxExtent(FVector(1, 1, 1));
	traceVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	traceVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	traceVolume->SetCollisionResponseToChannel(IMyTypes::Weapon_Srch, ECR_Overlap);
	traceVolume->SetHiddenInGame(true);

	rHandOverlap = CreateDefaultSubobject<USphereComponent>("Right hand overlap");
	rHandOverlap->SetSphereRadius(10.f);
	rHandOverlap->SetupAttachment(GetMesh(), "right_hand_socket");
	rHandOverlap->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	lHandOverlap = CreateDefaultSubobject<USphereComponent>("Left hand overlap");
	lHandOverlap->SetSphereRadius(10.f);
	lHandOverlap->SetupAttachment(GetMesh(), "left_hand_socket");
	lHandOverlap->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	rFootOverlap = CreateDefaultSubobject<USphereComponent>("Left foot overlap");
	rFootOverlap->SetSphereRadius(10.f);
	rFootOverlap->SetupAttachment(GetMesh(), "right_foot_socket");
	rFootOverlap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	rFootOverlap->SetCollisionResponseToAllChannels(ECR_Ignore);
	rFootOverlap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	UIWidgetBase = UI1.Class;

	GetCharacterMovement()->GroundFriction = 2;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048;

	isPlayer = true;
}

// Called when the game starts or when spawned
void AServant::BeginPlay()
{
	Super::BeginPlay();

	health = 600000;
	Tags.Add("Player");

	UIWidget = Cast<UServantUI>(CreateWidget<UUserWidget>(GetWorld(), UIWidgetBase));
	if (UIWidget)
	{
		UIWidget->AddToViewport();
	}

	myGameInstance = Cast<UServantGameInstance>(GetWorld()->GetGameInstance());

	if (myGameInstance)
	{
		nobleCache = myGameInstance->nobleCache;
	}

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AServant::OnPawnCollision);

	traceVolume->OnComponentBeginOverlap.AddDynamic(this, &AServant::OnOverlapBeginTrace);
	traceVolume->OnComponentEndOverlap.AddDynamic(this, &AServant::OnOverlapEndTrace);
	rHandOverlap->OnComponentBeginOverlap.AddDynamic(this, &AServant::OnOverlapBeginUnarmed);
	lHandOverlap->OnComponentBeginOverlap.AddDynamic(this, &AServant::OnOverlapBeginUnarmed);
	rFootOverlap->OnComponentBeginOverlap.AddDynamic(this, &AServant::OnOverlapBeginUnarmed);

	animInstance = Cast<UServantAnimInstance>(GetMesh()->GetAnimInstance());
}

#pragma endregion

// Called every frame
void AServant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (waitTime > 0)
	{
		ProcessWaitTime(DeltaTime);
		return;
	}

	if (activeAEOOverlaps.Num() > 0) HandleAOEDespawn(DeltaTime);

	if (isDodging) ProcessDodge(DeltaTime);
	if (isHoldingAttackInput) ProcessHoldInput(DeltaTime);

	if (isVolumeTracing) FloatTraceVolume(DeltaTime);
	if (isLineTracing) UpdateLineTrace();
	if (displayingTempWeapon) TimeoutTempDisplayWeapon(DeltaTime);

	//if (hitStopActive) HandleHitStop(DeltaTime);

	if (timeToConsumeBuffer > 0 && inputBuffer.Num() > 0) ProcessBufferAutoConsume(DeltaTime);

	if (lockedOn) OrientToLockedTarget(DeltaTime);
	if (forceMovementControl) ProcessMovementControl(DeltaTime);

	if (breakBowDestroy) ProcessBreakBowDestroy(DeltaTime);
	if (launchWeaponMode) ProcessLaunchWeaponMode(DeltaTime);

	if (climbStartCooldown > 0) ProcessClimbCooldown(DeltaTime);

	if (isClimbing)
	{
		if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Climbing", 10, 100);
		AdjustActorLocationForClimb(DeltaTime);
	}
	else
	{
		if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Not Climbing", 10, 100);
	}
}

#pragma region input

// Called to bind functionality to input
void AServant::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AServant::Jump);
	PlayerInputComponent->BindAction("Dodge", EInputEvent::IE_Pressed, this, &AServant::ParseDodgeInput);
	PlayerInputComponent->BindAction("ToggleCamera", EInputEvent::IE_Pressed, this, &AServant::ToggleGameMode);
	PlayerInputComponent->BindAction("PrimaryButton", EInputEvent::IE_Pressed, this, &AServant::ParsePrimaryInput);
	PlayerInputComponent->BindAction("PrimaryButton", EInputEvent::IE_Released, this, &AServant::ParseEndPrimaryInput);
	PlayerInputComponent->BindAction("SecondaryButton", EInputEvent::IE_Pressed, this, &AServant::ParseSecondaryInput);
	PlayerInputComponent->BindAction("TertiaryButton", EInputEvent::IE_Pressed, this, &AServant::ParseTertiaryInput);
	PlayerInputComponent->BindAction("SpecialButton1", EInputEvent::IE_Pressed, this, &AServant::ParseSpecialInput);
	PlayerInputComponent->BindAction("ShootButton", EInputEvent::IE_Pressed, this, &AServant::ParseShootInput);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AServant::EngageSprintMode);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AServant::DisengageSprintMode);
	PlayerInputComponent->BindAction("Noble", EInputEvent::IE_Pressed, this, &AServant::ParseNobleTraceCommand);
	PlayerInputComponent->BindAction("Lock", EInputEvent::IE_Pressed, this, &AServant::EstablishLock);
	PlayerInputComponent->BindAction("IncrementUp", EInputEvent::IE_Pressed, this, &AServant::IncrementTraceWeaponIndexUp);
	PlayerInputComponent->BindAction("IncrementDown", EInputEvent::IE_Pressed, this, &AServant::IncrementTraceWeaponIndexDown);

	PlayerInputComponent->BindAxis("MoveRight", this, &AServant::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AServant::MoveUp);
	PlayerInputComponent->BindAxis("LookUp", this, &AServant::AimUp);
	PlayerInputComponent->BindAxis("Turn", this, &AServant::AimRight);
	PlayerInputComponent->BindAxis("Increment", this, &AServant::IncrementTraceWeaponIndex);

}

void AServant::ParsePrimaryInput()
{
	if (isClimbing) return;
	isHoldingAttackInput = true;
	if (launchWeaponMode)
	{
		HandleNobleBreakMode();
	}
	else
	{
		if (currentGameMode == EGameMode::EGM_Standard)
		{
			ParseAttackCommand();
		}
		else if (currentGameMode == EGameMode::EGM_Trace)
		{
			if (!isLineTracing)
			{
				animInstance->TraceOn_Init();
			}
		}
	}

}

void AServant::ParseEndPrimaryInput()
{
	if (isClimbing) return;
	isHoldingAttackInput = false;
	currentAttackInputHoldTime = 0;
	if (currentGameMode == EGM_Trace)
	{
		animInstance->TraceOn_Cancel();
		if (isLineTracing)
		{
			CancelLineTrace();
		}
	}

}

void AServant::ParseHoldInput()
{
	if (isClimbing) return;
	if (currentGameMode == EGM_Standard)
	{
		ParseHoldAttackCommand();
		isHoldingAttackInput = false;
	}
}

void AServant::TryConsumeBufferInput()
{
	if (inputBuffer.Num() > 0)
	{
		animInstance->AttackCommand(true, 1);
		inputBuffer.RemoveAt(0);
	}
}

void AServant::ProcessBufferAutoConsume(float DeltaTime)
{
	timeToConsumeBuffer -= DeltaTime;

	if (timeToConsumeBuffer < 0)
	{
		TryConsumeBufferInput();
		timeToConsumeBuffer = .5;
	}
}

void AServant::ParseSecondaryInput()
{
	if (isClimbing) return;
	if (currentGameMode == EGameMode::EGM_Trace || currentGameMode == EGameMode::EGM_Standard)
	{
		TraceOn();
	}
}

void AServant::ParseTertiaryInput()
{
	if (isClimbing) return;
	if (currentGameMode == EGameMode::EGM_Trace || currentGameMode == EGameMode::EGM_Standard)
	{
		TraceOn_Launch();
	}
}

void AServant::ParseSpecialInput()
{
	if (isClimbing) return;
	if (currentGameMode == EGameMode::EGM_Trace && !isVolumeTracing)
	{
		InitTraceVolume();
	}
}

void AServant::ParseShootInput()
{
	if (isClimbing) return;
	if (currentGameMode == EGameMode::EGM_Trace || currentGameMode == EGameMode::EGM_Standard)
	{
		TryShootWeapon();
	}
}

void AServant::ParseDodgeInput()
{
	if (isClimbing) return;
	if (!canDodge || isDodging) return;

	if (FMath::IsNearlyZero(GetInputAxisValue("MoveForward")) &&
		FMath::IsNearlyZero(GetInputAxisValue("MoveRight")))
	{
		commitDodgeDir = UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0)) * -1;
	}
	else
	{
		commitDodgeDir = (UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0)) * GetInputAxisValue("MoveForward")) +
			(UKismetMathLibrary::GetRightVector(FRotator(0, GetControlRotation().Yaw, 0)) * GetInputAxisValue("MoveRight"));
		commitDodgeDir.Normalize();
	}

	int type = 0;

	if (FMath::Acos(FVector::DotProduct(GetActorForwardVector(), commitDodgeDir)) > 1.579)
	{
		type = 1;
	}

	animInstance->Dodge(type);
}

void AServant::IncrementTraceWeaponIndexUp()
{
	if (isClimbing) return;
	IncrementTraceWeaponIndex(1);
}

void AServant::IncrementTraceWeaponIndexDown()
{
	if (isClimbing) return;
	IncrementTraceWeaponIndex(-1);
}

void AServant::IncrementTraceWeaponIndex(float inval)
{
	if (isClimbing) return;
	if (currentGameMode == EGameMode::EGM_Trace || currentGameMode == EGameMode::EGM_Standard)
	{
		if (Controller && !(FMath::IsNearlyZero(inval)) && done_tracedWeapons.Num() > 0)
		{
			if (inval > 0)
			{
				if (traceWeaponIndex < done_tracedWeapons.Num() - 1)
				{
					traceWeaponIndex++;
				}
				else
				{
					traceWeaponIndex = 0;
				}
			}
			else
			{
				if (traceWeaponIndex > 0)
				{
					traceWeaponIndex--;
				}
				else
				{
					traceWeaponIndex = done_tracedWeapons.Num() - 1;
				}
			}
			SelectTracedWeapon();
		}
	}
}

void AServant::ParseNobleTraceCommand()
{
	if (isClimbing) return;
	if (isNobleTracing || (nobleCache.Num() < 1) || launchWeaponMode) return;

	if (personalWeapon)
	{
		if (personalWeapon->nobleWeapon)
		{
			HandleNobleBreakMode();
			return;
		}
	}

	if (crestPercentage < 1.f && !freeNobleSpawn) return;


	animInstance->TraceOn_Noble();
}

void AServant::ProcessHoldInput(float DeltaTime)
{
	if (isClimbing) return;
	currentAttackInputHoldTime += DeltaTime;

	if (currentAttackInputHoldTime > holdInputRegisterTime)
	{
		ParseHoldInput();
	}
}

#pragma endregion

#pragma region world

void AServant::ProcessWaitTime(float DeltaTime)
{
	waitTime -= DeltaTime;

	if (waitTime < 0)
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName("Test"));
	}
}

void AServant::ForceAIDoNothing(bool set)
{
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyServant::StaticClass(), foundActors);

	for (const auto foundActor : foundActors)
	{
		AEnemyServant* otherChar = Cast<AEnemyServant>(foundActor);
		otherChar->doNothing = set;
	}
}


#pragma endregion

#pragma region movement and aim

void AServant::ProcessMovementControl(float DeltaTime)
{
	if (!forceMovementCurve) return;
	timeInMovement += DeltaTime;

	AddActorWorldOffset((forceMovementCurve->GetVectorValue(timeInMovement).X * GetActorForwardVector() * DeltaTime) +
		(forceMovementCurve->GetVectorValue(timeInMovement).Y * GetActorRightVector() * DeltaTime) +
		(forceMovementCurve->GetVectorValue(timeInMovement).Z * GetActorUpVector() * DeltaTime)
		, true, nullptr, ETeleportType::TeleportPhysics);
}

void AServant::StartMovementControl(UCurveVector* inCurve, bool canIgnorePawn)
{
	if (!inCurve) return;

	forceMovementCounter++;

	if (canIgnorePawn) GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_MAX);
	forceMovementCurve = inCurve;
	timeInMovement = 0;
	forceMovementControl = true;

}

void AServant::EndMovementControl()
{
	forceMovementCounter--;
	if (launchWeaponMode) return;

	if (forceMovementCounter <= 0)
	{
		timeInMovement = 0;
		forceMovementControl = false;
		forceMovementCounter = 0;

		if (health > 0)
		{
			GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		}
	}

}

void AServant::ReportMovementInput()
{
	bool noMovementInput = (FMath::IsNearlyZero(GetInputAxisValue("MoveForward")) &&
		FMath::IsNearlyZero(GetInputAxisValue("MoveRight")));

	if (noMovementInput != lastResult)
	{
		lastResult = noMovementInput;
		animInstance->ReportMovementInput(lastResult);
	}
}

void AServant::SetMovementRestricted(bool isMoveRestrict, bool isAimRestrict, bool isDodgeRestrict)
{
	if (isMoveRestrict)
	{
		moveRestrictCounters++;
		movementRestricted = isMoveRestrict;
	}
	else
	{
		moveRestrictCounters--;
		if (moveRestrictCounters <= 0)
		{
			moveRestrictCounters = 0;
			movementRestricted = isMoveRestrict;
		}
	}

	aimRestricted = isAimRestrict;
	canDodge = !isDodgeRestrict;
}

void AServant::EngageSprintMode()
{
	if (sprintPressCount < 1)
	{
		isSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = (1500);
		animInstance->SetSprint(true);
	}
	sprintPressCount++;
}

void AServant::DisengageSprintMode()
{
	if (sprintPressCount > 1)
	{
		sprintPressCount = 0;
		isSprinting = false;
		GetCharacterMovement()->MaxWalkSpeed = (600);
		animInstance->SetSprint(false);
	}
}

void AServant::MoveUp(float axisval)
{
	if (currentGameMode == EGM_Cinematic || movementRestricted) return;

	if (isClimbing)
	{
		if (Controller && !(FMath::IsNearlyZero(axisval)))
		{
			float climbSpeed = baseClimbSpeed * (isSprinting ? 2 : 1);

			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Move Up", 1, 111);
			//FVector wallAdjustedMove = FVector::CrossProduct(anchorHit.ImpactNormal, -GetActorRightVector()) * (axisval > 0 ? 1 : -1);
			FVector wallAdjustedMove = GetCapsuleComponent()->GetUpVector() * (axisval > 0 ? 1 : -1);
			AddActorWorldOffset(wallAdjustedMove * climbSpeed * GetWorld()->GetDeltaSeconds(), false, nullptr, ETeleportType::TeleportPhysics);
			climbingMove.X = axisval;
		}
		else
		{
			climbingMove.X = 0;
		}
	}
	else
	{
		if (Controller && !(FMath::IsNearlyZero(axisval)))
		{
			if (currentGameMode == EGM_Standard) AddMovementInput(UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0)), axisval);
			else AddMovementInput(GetActorForwardVector(), axisval);
		}
		else
		{
			if (GetCharacterMovement()->GetLastUpdateVelocity().IsNearlyZero(.1) && isSprinting)
			{
				sprintPressCount = 2;
				DisengageSprintMode();
			}
		}
	}

}

void AServant::MoveRight(float axisval)
{
	if (currentGameMode == EGameMode::EGM_Cinematic || movementRestricted) return;

	if (isClimbing)
	{
		if (Controller && !(FMath::IsNearlyZero(axisval)))
		{
			float climbSpeed = baseClimbSpeed * (isSprinting ? 2 : 1);

			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Move Right", 1, 111);
			//FVector wallAdjustedMove = FVector::CrossProduct(anchorHit.ImpactNormal, GetActorUpVector()) * (axisval > 0 ? 1 : -1);
			FVector wallAdjustedMove = GetCapsuleComponent()->GetRightVector() * (axisval > 0 ? 1 : -1);
			AddActorWorldOffset(wallAdjustedMove * climbSpeed * GetWorld()->GetDeltaSeconds(), false, nullptr, ETeleportType::TeleportPhysics);

			climbingMove.Y = axisval;
		}
		else
		{
			climbingMove.Y = 0;
		}
	}
	else
	{
		if (Controller && !(FMath::IsNearlyZero(axisval)))
		{
			if (currentGameMode == EGM_Standard) AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0, GetControlRotation().Yaw, 0)), axisval);
			else AddMovementInput(GetActorRightVector(), axisval);
		}
		else
		{
			if (GetCharacterMovement()->GetLastUpdateVelocity().IsNearlyZero(.1) && isSprinting)
			{
				sprintPressCount = 2;
				DisengageSprintMode();
			}
		}
	}

}

void AServant::AimUp(float axisval)
{

	if (isInterpCameraPos || currentCameraSetting == ECameraMode::ECM_Cinematic) return;
	if (Controller && !(FMath::IsNearlyZero(axisval)))
	{
		if (currentCameraSetting == ECameraMode::ECM_Standard)
		{
			AddControllerPitchInput(axisval);
		}
		else if (currentCameraSetting == ECameraMode::ECM_Trace)
		{
			float pitch = (.5 * axisval * cameraInversion);
			camera->AddLocalRotation(FRotator(FMath::Clamp(pitch, -20.f, 40.f), 0, 0));
		}
	}
}

void AServant::AimRight(float axisval)
{

	if (isInterpCameraRot || currentCameraSetting == ECameraMode::ECM_Cinematic) return;
	if (Controller && !(FMath::IsNearlyZero(axisval)))
	{
		if (lockedOn)
		{
			if (lockCooldown > 0) return;
			if (FMath::Abs(axisval) > .95)
			{
				lockedTarget = Cast<AServant>(FindClosestActorInDirectionByAngle(axisval < 0));
				lockCooldown = .2;
			}
			return;
		}
		
		if (currentCameraSetting == ECameraMode::ECM_Standard)
		{
			AddControllerYawInput(axisval);
		}
		else if (currentCameraSetting == ECameraMode::ECM_Trace)
		{
			AddControllerYawInput(axisval);
		}
	}
}

void AServant::Jump()
{
	if (currentGameMode == EGameMode::EGM_Cinematic || movementRestricted) return;

	if (isClimbing)
	{
		if (GetInputAxisValue("MoveForward") < -.95)
		{
			TryJumpOffWall();
		}
	}
	else
	{
		Super::Jump();
	}

}

void AServant::StartDodge()
{
	isDodging = true;
	dodgeTime = 0;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_MAX);

	ProcessDodge(GetWorld()->GetDeltaSeconds());
}

void AServant::ProcessDodge(float DeltaTime)
{
	dodgeTime += DeltaTime;
	AddActorWorldOffset((dodgeCurve->GetVectorValue(dodgeTime).X * commitDodgeDir * DeltaTime), true, nullptr, ETeleportType::TeleportPhysics);
}

void AServant::EndDodge()
{
	isDodging = false;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
}

void AServant::SetIFrame(bool inInvincible)
{
	invincible = inInvincible;
	noHit = inInvincible;
}

float AServant::IKFootTrace(const FName &inSocketName, float traceDist, bool drawDebug)
{
	FVector footLoc = GetMesh()->GetSocketLocation(inSocketName);

	FVector startLoc = FVector(footLoc.X, footLoc.Y, GetActorLocation().Z);
	FVector endLoc = FVector(footLoc.X, footLoc.Y, GetActorLocation().Z - traceDist);

	FVector startToFoot = footLoc - startLoc;
	FVector startToHit = FVector::ZeroVector;

	FHitResult outHit = FHitResult();
	FVector ret = FVector::ZeroVector;

	FCollisionQueryParams myQ = FCollisionQueryParams();
	myQ.bTraceComplex = true;

	GetWorld()->LineTraceSingleByChannel(outHit, startLoc, endLoc, ECollisionChannel::ECC_Visibility, myQ, FCollisionResponseParams(ECR_Block));
	int multFactor = 1;

	if (outHit.IsValidBlockingHit())
	{
		startToHit = outHit.Location - startLoc;

		if (startToHit.Size() > startToFoot.Size()) multFactor = -1;
		ret = outHit.Location - footLoc;
	}

	if (drawDebug)
	{

		if (outHit.IsValidBlockingHit())
		{
			IMyTypes::Debug_Print(inSocketName.ToString() + " Offset: " + (multFactor > 0 ? "" : "-") + FString::SanitizeFloat(ret.Size()) +
				" Start to foot: " + FString::SanitizeFloat(startToFoot.Size()) + " Start to hit: " + FString::SanitizeFloat(startToHit.Size()), 1);

			DrawDebugLine(GetWorld(), startLoc, endLoc, FColor::Blue, false, 0);
			DrawDebugLine(GetWorld(), startLoc, footLoc, FColor::Red, false, 0);
			DrawDebugLine(GetWorld(), startLoc, outHit.Location, FColor::Purple, false, 0);
		}
		else IMyTypes::Debug_Print(inSocketName.ToString() + " Miss", 1);
	}

	if (multFactor == -1 || ret.Size() == 0)
	{
		return ret.Size() * multFactor;
	}
	else
	{
		return ret.Size() + 10;
	}

}

#pragma region climbing

#pragma region climb init

void AServant::OnPawnCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult &Hit)
{

	if (OtherComp->GetCollisionObjectType() == IMyTypes::ECC_CLIMB && !isClimbing && climbStartCooldown <= 0)
	{
		climbStartCooldown = .5;
		if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Hit climbable object, attempt to climb start", 10, 100);
		AttemptBeginClimb(OtherComp, Hit.ImpactPoint, Hit);
	}
	else if ((OtherComp->GetCollisionObjectType() == ECC_WorldStatic) && isClimbing && climbStartCooldown <= 0)
	{
		if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Hit floor, attempting to abort climb", 10, 100);
		FHitResult myHit = FHitResult();
		GetWorld()->LineTraceSingleByObjectType(myHit, GetCapsuleComponent()->GetComponentLocation(), GetCapsuleComponent()->GetComponentLocation() - GetCapsuleComponent()->GetUpVector() * 150, ECC_WorldStatic);
		if (myHit.IsValidBlockingHit())
		{
			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Climb aborted by floor hit", 10, 100);
			ChangeClimbMovementMode(false);
		}
	}
}

void AServant::ChangeClimbMovementMode(bool climb)
{
	if (climb)
	{
		if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Set climb mode true", 10, 101);
		ResetCameraToStandard();

		if (personalWeapon)
		{
			personalWeapon->GetWeaponMesh()->SetHiddenInGame(true);
			personalWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "preview_socket");
			personalWeapon->UseWeaponAsActorOrientation(GetMesh()->GetSocketLocation("preview_socket_point") - GetMesh()->GetSocketLocation("preview_socket"));
			personalWeapon->KickOffSpawnSequence();
		}

		animInstance->StartStopClimb(true);
		GetCharacterMovement()->MaxWalkSpeed = (500);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_MAX);
		isClimbing = true;
		//bUseControllerRotationYaw = true;
		//GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Set climb mode false", 10, 101);
		animInstance->StartStopClimb(false);
		GetCharacterMovement()->MaxWalkSpeed = (600);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

		if (personalWeapon)
		{
			personalWeapon->GetWeaponMesh()->SetHiddenInGame(true);
			personalWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "right_hand_socket");
			personalWeapon->UseWeaponAsActorOrientation(GetMesh()->GetSocketLocation("weapon_point_socket") - GetMesh()->GetSocketLocation("right_hand_socket"));
			personalWeapon->KickOffSpawnSequence();
		}

		isClimbing = false;
		climbStartCooldown = .5;

		SetActorRotation(FRotator(0, GetActorRotation().Yaw, 0));
		//bUseControllerRotationYaw = false;
		//GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void AServant::ProcessClimbCooldown(float DeltaTime)
{
	if (climbStartCooldown > 0)	climbStartCooldown -= DeltaTime;
}

void AServant::AttemptBeginClimb(UPrimitiveComponent* compToClimb, FVector loc, const FHitResult &hit)
{
	if (isClambering) return;

	FVector dirToClimbSurface = (loc - GetActorLocation()) * FVector(1, 1, 0);

	FVector2D targetDetails = FMath::GetAzimuthAndElevation(dirToClimbSurface, 
		GetCapsuleComponent()->GetForwardVector(), GetCapsuleComponent()->GetRightVector(), GetCapsuleComponent()->GetUpVector());
	float azimuth = FMath::Abs(targetDetails.X);

	if (climbDebugVerbosity > 0)
	{
		IMyTypes::Debug_Print("Attempt begin climb, target azimuth: " + FString::SanitizeFloat(azimuth), 10, 102);
		if (climbDebugVerbosity > 1)
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (GetCapsuleComponent()->GetForwardVector() * 100), FColor::Yellow, false, 1);
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (dirToClimbSurface.GetSafeNormal() * 100), FColor::Yellow, false, 1);
			DrawDebugSphere(GetWorld(), loc, 10, 5, FColor::Yellow, false, 1);
		}
	}

	if (azimuth < 1.042)
	{
		if (SetClimbingMatrix(GetWorld()->GetDeltaSeconds()))
		{
			ChangeClimbMovementMode(true);
		}
	}
	else
	{
		if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Attempt begin climb failed, target azimuth too great: " + FString::SanitizeFloat(azimuth), 10, 102);
	}

}

#pragma endregion

#pragma region climb utility

bool AServant::SetClimbingMatrix(float DeltaTime)
{
	FVector acLoc = GetActorLocation();
	FVector acFwd = GetCapsuleComponent()->GetForwardVector();
	FVector acR = GetCapsuleComponent()->GetRightVector();
	FVector acU = GetCapsuleComponent()->GetUpVector();

	FVector rightHandLoc = GetMesh()->GetSocketLocation(FName("hand_r"));
	FVector leftHandLoc = GetMesh()->GetSocketLocation(FName("hand_l"));
	FVector rightFootLoc = GetMesh()->GetSocketLocation(FName("foot_r"));
	FVector leftFootLoc = GetMesh()->GetSocketLocation(FName("foot_l"));

	FVector rHTrgt = rightHandLoc;
	FVector lHTrgt = leftHandLoc;

	rightHandHorizOffset = rightHandLoc;
	leftHandHorizOffset = leftHandLoc;
	rightFootHorizOffset = rightFootLoc;
	leftFootHorizOffset = leftFootLoc;

	if (rightHandLoc == FVector::ZeroVector || leftHandLoc == FVector::ZeroVector || rightFootLoc == FVector::ZeroVector || leftFootLoc == FVector::ZeroVector)
	{
		if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Could not find bones, matrix abort", 10, 106);
		return false;
	}

	if (!TrySetClimbingAnchor())
	{
		if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Failed to set anchor", 10, 106);
		return false;
	}

	bool findRight = TryFindHit(rightHandHit, rightHandHorizOffset, rightHandLoc, -acR, -anchorHit.ImpactNormal);
	bool findLeft = TryFindHit(leftHandHit, leftHandHorizOffset, leftHandLoc, acR, -anchorHit.ImpactNormal);

	if (!findLeft && !findRight)
	{
		if (AttemptClamber())
		{
			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Hand fail, attempt clamber", 10, 106);
			TryClamber();
		}
		else
		{
			if (!findLeft && !findRight)
			{
				if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Both hands fail, matrix abort", 10, 106);
				return false;
			}
		}
	}

	TryFindHit(rightFootHit, rightFootHorizOffset, rightFootLoc, -acR, -anchorHit.ImpactNormal);
	TryFindHit(leftFootHit, leftFootHorizOffset, leftFootLoc, acR, -anchorHit.ImpactNormal);

	IKClimbTrace(FName("hand_r"), rightHandHorizOffset);
	IKClimbTrace(FName("hand_l"), leftHandHorizOffset);
	IKClimbTrace(FName("foot_r"), rightFootHorizOffset);
	IKClimbTrace(FName("foot_l"), leftFootHorizOffset);

	MapInputToClimbMovement();

	return true;
}

bool AServant::TrySetClimbingAnchor()
{
	FVector acLoc = GetActorLocation();
	FVector acFwd = GetCapsuleComponent()->GetForwardVector();
	FVector acUp = GetCapsuleComponent()->GetUpVector();
	FVector acRght = GetCapsuleComponent()->GetRightVector();

	GetWorld()->LineTraceSingleByChannel(anchorHit, acLoc, acLoc + (acFwd * maxClimbSurfaceDistance), IMyTypes::ECC_CLIMB_TRACE);

	if (climbDebugVerbosity > 1)
	{
		DrawDebugLine(GetWorld(), acLoc, acLoc + (acFwd * maxClimbSurfaceDistance), FColor::Orange, false, 0);
	}

	if (!anchorHit.IsValidBlockingHit())
	{

		int direction = GetInputAxisValue("MoveRight") > 0 ? 1 : -1;
		FVector dspAdjustAcLoc = acLoc + (acRght * 25 * direction);
		FVector angleAdjustFwd = acFwd.RotateAngleAxis(-35.f * direction, GetCapsuleComponent()->GetUpVector());

		if (!FMath::IsNearlyZero(GetInputAxisValue("MoveRight")))
		{
			if (climbDebugVerbosity > 1)
			{
				DrawDebugLine(GetWorld(), dspAdjustAcLoc, dspAdjustAcLoc + (angleAdjustFwd * maxClimbSurfaceDistance), FColor::Orange, false, 0);
			}
			GetWorld()->LineTraceSingleByChannel(anchorHit, dspAdjustAcLoc, dspAdjustAcLoc + (angleAdjustFwd * maxClimbSurfaceDistance), IMyTypes::ECC_CLIMB_TRACE);

			if (!anchorHit.IsValidBlockingHit())
			{
				return false;
			}
		}
		else
		{
			GetWorld()->LineTraceSingleByChannel(anchorHit, dspAdjustAcLoc, dspAdjustAcLoc + (angleAdjustFwd * maxClimbSurfaceDistance), IMyTypes::ECC_CLIMB_TRACE);
			if (climbDebugVerbosity > 1)
			{
				DrawDebugLine(GetWorld(), dspAdjustAcLoc, dspAdjustAcLoc + (angleAdjustFwd * maxClimbSurfaceDistance), FColor::Orange, false, 10);
			}
			if (!anchorHit.IsValidBlockingHit())
			{
				dspAdjustAcLoc = acLoc + (acRght * 25 * -direction);
				angleAdjustFwd = acFwd.RotateAngleAxis(-35.f * direction * -1, GetCapsuleComponent()->GetUpVector());
				GetWorld()->LineTraceSingleByChannel(anchorHit, dspAdjustAcLoc, dspAdjustAcLoc + (angleAdjustFwd * maxClimbSurfaceDistance), IMyTypes::ECC_CLIMB_TRACE);
				if (climbDebugVerbosity > 1)
				{
					DrawDebugLine(GetWorld(), dspAdjustAcLoc, dspAdjustAcLoc + (angleAdjustFwd * maxClimbSurfaceDistance), FColor::Orange, false, 0);
				}
				if (!anchorHit.IsValidBlockingHit())
				{
					return false;
				}
			}
		}
	}

	if (climbDebugVerbosity > 1)
	{
		DrawDebugSphere(GetWorld(), anchorHit.ImpactPoint, 10, 5, FColor::Orange, false, 0);
		DrawDebugLine(GetWorld(), acLoc, acLoc + (acFwd * maxClimbSurfaceDistance), FColor::Orange, false, 0);
	}

	return true;
}

bool AServant::TryFindHit(FHitResult &hit, FVector &horizOffset, FVector searchStart, FVector searchDir, FVector probeDir, int tries)
{
	if (tries <= 0 || tries > 5) tries = 3;
	searchDir.Normalize();
	hit = FHitResult();

	bool found = false;

	searchStart = searchStart + (probeDir * -25);

	GetWorld()->LineTraceSingleByChannel(hit, searchStart, searchStart + (probeDir * maxClimbSurfaceDistance), IMyTypes::ECC_CLIMB_TRACE);

	if (hit.IsValidBlockingHit())
	{
		if (climbDebugVerbosity > 1)
		{
			DrawDebugSphere(GetWorld(), hit.ImpactPoint, 10, 5, FColor::Emerald, false, 0);
			DrawDebugLine(GetWorld(), searchStart, searchStart + (probeDir * maxClimbSurfaceDistance), FColor::Emerald, false, 0);
		}
		horizOffset = hit.ImpactPoint;
		return true;
	}

	int durMult = 1;

	for (int i = 1; i < (tries*2) + 1; i++)
	{
		durMult *= -1;
		FVector placeToSearch = searchStart + (searchDir * durMult * (maxDeltaDist / tries) * (i%tries));
		FVector dirToSearch = placeToSearch + probeDir * maxClimbSurfaceDistance;

		GetWorld()->LineTraceSingleByChannel(hit, placeToSearch, dirToSearch, IMyTypes::ECC_CLIMB_TRACE);
		if (hit.IsValidBlockingHit())
		{
			if (climbDebugVerbosity > 1)
			{
				DrawDebugSphere(GetWorld(), hit.ImpactPoint, 10, 5, FColor::Emerald, false, 0);
				DrawDebugLine(GetWorld(), placeToSearch, dirToSearch, FColor::Emerald, false, 0);
			}

			horizOffset = hit.ImpactPoint;
			found = true;
			break;
		}
		else
		{
			if (climbDebugVerbosity > 1) DrawDebugLine(GetWorld(), placeToSearch, dirToSearch, FColor::Red, false, 0);
		}
	}

	if (!found) return false;
	return true;
}

void AServant::AdjustActorLocationForClimb(float DeltaTime)
{
	if (!SetClimbingMatrix(DeltaTime))
	{
		if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Adjust location for climb fail", 10, 103);
		ChangeClimbMovementMode(false);
		return;
	}

	AdjustActorClimbingLocation(DeltaTime);
	AdjustActorClimbingRotation(DeltaTime);
}

void AServant::AdjustActorClimbingLocation(float DeltaTime)
{
	anchorChord = anchorHit.ImpactPoint - GetActorLocation();
	FVector desiredAnchorLocation = anchorHit.ImpactPoint + anchorHit.ImpactNormal * desiredAnchorOffset;
	FVector actorToDesiredAnchor = desiredAnchorLocation - GetActorLocation();

	if (climbDebugVerbosity > 0)
	{
		IMyTypes::Debug_Print("Distance from desired anchor: " + FString::SanitizeFloat(actorToDesiredAnchor.Size()) +
			"/" + FString::SanitizeFloat(desiredAnchorDist) + " Overflow: " + FString::FromInt(isAnchorOverflow), 10, 98);
	}

	if (!isAnchorOverflow)
	{
		if (actorToDesiredAnchor.Size() > desiredAnchorDist)
		{
			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Adjust climber to anchor positon" + FString::SanitizeFloat(anchorChord.Size()), 10, 104);
			SetActorLocation(FMath::VInterpTo(GetActorLocation(), desiredAnchorLocation, DeltaTime, 10));

			if (climbDebugVerbosity > 0)
			{
				DrawDebugLine(GetWorld(), GetActorLocation(), desiredAnchorLocation, FColor::Black, false, 0);
				DrawDebugSphere(GetWorld(), desiredAnchorLocation, 10, 5, FColor::Black, false, 0);
			}
		}
		else
		{
			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Anchor start overflow" + FString::SanitizeFloat(anchorChord.Size()), 10, 104);
			isAnchorOverflow = true;

			if (climbDebugVerbosity > 0)
			{
				DrawDebugLine(GetWorld(), GetActorLocation(), desiredAnchorLocation, FColor::Green, false, 0);
				DrawDebugSphere(GetWorld(), desiredAnchorLocation, 10, 5, FColor::Green, false, 0);
			}
		}


	}
	else
	{
		if (actorToDesiredAnchor.Size() > desiredAnchorDist + anchorOverflowLimit)
		{
			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Anchor end overflow" + FString::SanitizeFloat(anchorChord.Size()), 10, 104);
			isAnchorOverflow = false;
		}

		if (climbDebugVerbosity > 0)
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), desiredAnchorLocation, FColor::Green, false, 0);
			DrawDebugSphere(GetWorld(), desiredAnchorLocation, 10, 5, FColor::Green, false, 0);
		}
	}
}

void AServant::AdjustActorClimbingRotation(float DeltaTime)
{
	FVector wallDir = anchorHit.ImpactNormal * -1;
	FQuat wallRot = wallDir.ToOrientationQuat();

	FVector directionToAnchor = -anchorHit.ImpactNormal;

	FVector2D targetDetails = FMath::GetAzimuthAndElevation(directionToAnchor,
		GetCapsuleComponent()->GetForwardVector(), GetCapsuleComponent()->GetRightVector(), GetCapsuleComponent()->GetUpVector());
	float elevation = FMath::RadiansToDegrees(FMath::Abs(targetDetails.Y));
	float azimuth = FMath::RadiansToDegrees(FMath::Abs(targetDetails.X));

	if (climbDebugVerbosity > 0)
	{
		IMyTypes::Debug_Print("Relative Elevation: " + FString::SanitizeFloat(elevation) + "/" + FString::SanitizeFloat(desiredAnchorRotAccuracy) 
			+ " Overflow : " + FString::FromInt(isRotOverflow) + ", Azimuth: " + FString::SanitizeFloat(azimuth) + "/" + FString::SanitizeFloat(desiredAnchorRotAccuracy)
			+ " Overflow : " + FString::FromInt(isRotOverflowAzimuth), 10, 99);
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (directionToAnchor.GetSafeNormal() * 100), FColor::Purple, false, 0);
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (GetCapsuleComponent()->GetForwardVector() * 100), FColor::Purple, false, 0);
	}

	if (!isRotOverflowAzimuth)
	{
		if (azimuth > desiredAnchorRotAccuracy)
		{
			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Target angle azimuth too great, adjust angle to match. Azimuth: " + FString::SanitizeFloat(azimuth), 10, 105);
			SetActorRotation(FMath::QInterpTo(GetActorQuat(), wallRot, DeltaTime, 10));
		}
		else
		{
			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Rot Azimuth begin overflow", 10, 105);
			isRotOverflowAzimuth = true;
		}
	}
	else
	{
		if (azimuth > desiredAnchorRotAccuracy + achorRotAccuracyOverflowLimit)
		{
			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Rot Azimuth end overflow", 10, 105);
			isRotOverflowAzimuth = false;
		}
	}

	if (!isRotOverflow)
	{
		if (elevation > desiredAnchorRotAccuracy)
		{
			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Target angle elevation too great, adjust angle to match. Elevation: " + FString::SanitizeFloat(elevation), 10, 105);
			SetActorRotation(FMath::QInterpTo(GetActorQuat(), wallRot, DeltaTime, 1));
		}
		else
		{
			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Rot begin overflow", 10, 105);
			isRotOverflow = true;
		}
	}
	else
	{
		if (elevation > desiredAnchorRotAccuracy + achorRotAccuracyOverflowLimit)
		{
			if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Rot end overflow", 10, 105);
			isRotOverflow = false;
		}
	}

}

bool AServant::AttemptClamber()
{
	FVector acLoc = GetActorLocation();
	FVector acFwd = GetCapsuleComponent()->GetForwardVector();
	FVector acR = GetCapsuleComponent()->GetRightVector();
	FVector acU = GetCapsuleComponent()->GetUpVector();

	FVector rightHandLoc = GetMesh()->GetBoneLocation(FName("VB upperarm_r_hand_r"), EBoneSpaces::WorldSpace);
	FVector leftHandLoc = GetMesh()->GetBoneLocation(FName("VB upperarm_l_hand_l"), EBoneSpaces::WorldSpace);

	rightHandHorizOffset = rightHandLoc;
	leftHandHorizOffset = leftHandLoc;

	bool findLeft = TryFindHit(leftHandHit, leftHandHorizOffset, leftHandLoc + acFwd * 30, -acR, -acU);
	bool findRight = TryFindHit(rightHandHit, rightHandHorizOffset, rightHandLoc + acFwd * 30, -acR, -acU);

	if (findLeft || findRight)
	{
		return true;
		if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Can clamber", 10, 115);
	}
	else
	{
		if (climbDebugVerbosity > 0) IMyTypes::Debug_Print("Cannot clamber", 10, 115);
		return false;
	}
}

float AServant::IKClimbTrace(const FName &inSocketName, FVector &offSetVector)
{
	FVector startLoc = offSetVector + (anchorHit.ImpactNormal * 50);
	FVector endLoc = startLoc + (anchorHit.ImpactNormal * maxClimbSurfaceDistance * -1);

	FHitResult outHit = FHitResult();
	FVector ret = FVector::ZeroVector;

	FCollisionQueryParams myQ = FCollisionQueryParams();
	myQ.bTraceComplex = true;

	GetWorld()->LineTraceSingleByChannel(outHit, startLoc, endLoc, ECollisionChannel::ECC_Visibility, myQ, FCollisionResponseParams(ECR_Block));
	if (climbDebugVerbosity > 1) DrawDebugLine(GetWorld(), startLoc, endLoc, FColor::Blue, false, 0);

	if (outHit.IsValidBlockingHit())
	{
		offSetVector = outHit.ImpactPoint;
		if (climbDebugVerbosity > 2)
		{
			DrawDebugSphere(GetWorld(), outHit.ImpactPoint, 10, 5, FColor::Blue, false, 0);
		}
	}

	return 1;
}

void AServant::MapInputToClimbMovement()
{
	animInstance->SetClimbDirection(climbingMove);
	animInstance->SetBaseIKLocators(rightHandHorizOffset, leftHandHorizOffset, rightFootHorizOffset, leftFootHorizOffset);
}

#pragma endregion

#pragma region climbing actions

void AServant::TryJumpOffWall()
{
	isClimbing = false;
	climbStartCooldown = 1;
	animInstance->JumpFromWall();

	ChangeClimbMovementMode(false);
}

void AServant::DoJumpOffWall()
{

}

void AServant::TryClamber()
{
	isClimbing = false;
	climbStartCooldown = 1;

	animInstance->Clamber();
	ChangeClimbMovementMode(false);
}

#pragma endregion

#pragma endregion

#pragma endregion

#pragma region game mode

void AServant::ToggleGameMode()
{
	if (isClimbing) return;
	if (launchWeaponMode) return;

	if (lockedOn) EstablishLock();

	if (currentGameMode == EGameMode::EGM_Standard)
	{
		SetGameMode(EGameMode::EGM_Trace);
	}
	else if (currentGameMode == EGameMode::EGM_Trace)
	{
		SetGameMode(EGameMode::EGM_Standard);
		CancelLineTrace();
	}
}

void AServant::SetGameMode(EGameMode newGameMode)
{
	currentGameMode = newGameMode;

	if (newGameMode == EGameMode::EGM_Cinematic)
	{
		DestroyTempDisplayWeapon();
	}
	else
	{
		if (isNobleTracing) return;
		ToggleCamera();
	}
}

#pragma endregion

#pragma region camera

void AServant::ToggleCamera()
{
	if (launchWeaponMode) return;
	if (currentCameraSetting == ECameraMode::ECM_Standard)
	{
		SetCameraMode(ECameraMode::ECM_Trace);
	}
	else if (currentCameraSetting == ECameraMode::ECM_Trace)
	{
		SetCameraMode(ECameraMode::ECM_Standard);
	}
}

void AServant::SetCameraMode(ECameraMode newCameraMode)
{
	if (newCameraMode == ECameraMode::ECM_Trace)
	{
		animInstance->TraceMode(true);
		currentCameraSetting = newCameraMode;

		targetingCameraBoom->SetRelativeRotation(FRotator(-20, 0, 0));

		camera->AttachToComponent(targetingCameraBoom, FAttachmentTransformRules::SnapToTargetIncludingScale, USpringArmComponent::SocketName);
		targetingCameraBoom->bEnableCameraLag = true;

		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;

	}
	else if (newCameraMode == ECameraMode::ECM_Standard)
	{
		animInstance->TraceMode(false);
		currentCameraSetting = newCameraMode;

		cameraBoom->SetRelativeRotation(FRotator(-20, 0, 0));

		camera->AttachToComponent(cameraBoom, FAttachmentTransformRules::SnapToTargetIncludingScale, USpringArmComponent::SocketName);
		cameraBoom->bEnableCameraLag = true;

		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;

		SetActorRotation(FRotator(0,GetBaseAimRotation().Yaw, 0));
		GetController()->SetControlRotation(FRotator(-20,GetActorForwardVector().ToOrientationRotator().Yaw,0));

	}
	else if (newCameraMode == ECameraMode::ECM_Cinematic)
	{
		animInstance->TraceMode(false);
		currentCameraSetting = newCameraMode;
		camera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	}
}

void AServant::ResetCamera()
{
	cameraBoom->SetRelativeRotation(FRotator(-20, 0, 0));
	desiredCameraPitch = 0;
}

void AServant::StartNobleCinematic()
{
	if (lockedOn) EstablishLock();
	isNobleTracing = true;
	SetCameraMode(ECM_Standard);
	SetGameMode(EGM_Standard);
}

void AServant::ProcessNobleCinematic(float DeltaTime)
{
	InterpCameraNoble(DeltaTime);
}

void AServant::InterpCameraNoble(float DeltaTime)
{
	camera->AddWorldOffset(camera->GetRightVector() * 9);
	//camera->SetWorldLocation(FVector(camera->GetComponentLocation().X, camera->GetComponentLocation().Y, GetMesh()->GetComponentLocation().Z));
	camera->SetWorldRotation((GetMesh()->GetComponentLocation() - camera->GetComponentLocation()).ToOrientationQuat());
}

void AServant::EndNobleCinematic()
{
	isNobleTracing = false;
}

void AServant::InterpCameraAim(float DeltaTime)
{
	float pitch = cameraBoom->GetRelativeTransform().Rotator().Pitch;
	if (!FMath::IsNearlyEqual(pitch, desiredCameraPitch, .05f))
	{
		cameraBoom->SetRelativeRotation(FRotator(FMath::FInterpTo(pitch, desiredCameraPitch, DeltaTime, 2), 0, 0));
	}
	else
	{
		isInterpCameraAim = false;
		desiredCameraPitch = 0;
	}
}

void AServant::InterpCameraPos(float DeltaTime)
{
	FVector pos = camera->GetRelativeLocation();
	if (!FVector::PointsAreNear(pos, desiredCameraPos, 1.f))
	{
		camera->SetRelativeLocation(FMath::VInterpTo(pos, desiredCameraPos, DeltaTime, 10));
	}
	else
	{
		if (currentCameraSetting == ECM_Standard)
		{
			camera->AttachToComponent(cameraBoom, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
		}
		else if (currentCameraSetting == ECM_Trace)
		{
			//camera->AttachToComponent(targetingCameraBoom, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
			camera->SetWorldRotation(GetActorForwardVector().ToOrientationQuat());
		}

		isInterpCameraPos = false;
		isInterpCameraRot = false;
	}

}

void AServant::InterpCameraRot(float DeltaTime)
{
	FRotator rot = camera->GetRelativeTransform().Rotator();

	if (!rot.Equals(desiredCameraRot, .5f))
	{
		camera->SetRelativeRotation(FMath::RInterpTo(rot, desiredCameraRot, DeltaTime, 8));
	}
	else 
	{
		isInterpCameraRot = false;
	}
}

void AServant::ResetCameraToStandard()
{
	if (currentGameMode == ECM_Trace)
	{
		SetGameMode(EGameMode::EGM_Standard);
		CancelLineTrace();
	}
}

#pragma endregion

#pragma region lock

AActor* AServant::FindClosestActor()
{
	TArray<AActor*> foundActors;

	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AServant::StaticClass(), FName("Targetable"), foundActors);
	float minDist = FLT_MAX;

	AActor* retActor = nullptr;

	for (int i = 0; i < foundActors.Num(); i++)
	{
		if (foundActors[i] == lockedTarget) continue;
		FVector dirTo = foundActors[i]->GetActorLocation() - GetActorLocation();
		if (dirTo.Size() < minDist)
		{
			minDist = dirTo.Size();
			retActor = foundActors[i];
		}
	}

	return retActor;
}

AActor* AServant::FindClosestActorInDirection(bool left)
{
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AServant::StaticClass(), FName("Targetable"), foundActors);

	float minDist = FLT_MAX;
	AActor* retActor = nullptr;

	for (int i = 0; i < foundActors.Num(); i++)
	{
		AActor* myActor = foundActors[i];
		if (!myActor) continue;
		if (myActor == lockedTarget) continue;
		FVector dirTo = myActor->GetActorLocation() - GetActorLocation();

		FVector saveDirTarget = dirTo * FVector(1,1,0);
		saveDirTarget.Normalize();

		FVector2D targetDetails = FMath::GetAzimuthAndElevation(saveDirTarget, GetActorForwardVector(), GetActorRightVector(), GetActorUpVector());
		float azimuth = targetDetails.X;

		if (azimuth < 0 && left) continue;
		else if (azimuth > 0 && !left) continue;

		if (dirTo.Size() < minDist)
		{
			minDist = dirTo.Size();
			retActor = myActor;
		}
	}

	if (!retActor)
	{
		retActor = FindClosestActor();
		if (!retActor)
		{
			return lockedTarget;
		}
	}
	return retActor;
}

AActor* AServant::FindClosestActorInDirectionByAngle(bool left, bool either)
{
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AServant::StaticClass(), FName("Targetable"), foundActors);

	float minAngle = FLT_MAX;
	AActor* retActor = nullptr;

	for (int i = 0; i < foundActors.Num(); i++)
	{
		AActor* myActor = foundActors[i];
		if (!myActor) continue;
		if (myActor == lockedTarget) continue;
		FVector dirTo = myActor->GetActorLocation() - GetActorLocation();

		FVector saveDirTarget = dirTo * FVector(1, 1, 0);
		saveDirTarget.Normalize();

		FVector2D targetDetails = FMath::GetAzimuthAndElevation(saveDirTarget, GetActorForwardVector(), GetActorRightVector(), GetActorUpVector());
		float azimuth = targetDetails.X;

		if (azimuth > 0 && left && !either) continue;
		else if (azimuth < 0 && !left && !either) continue;

		azimuth = FMath::Abs(azimuth);

		if (azimuth < minAngle)
		{
			minAngle = azimuth;
			retActor = myActor;
		}
	}

	if (!retActor)
	{
		retActor = FindClosestActor();
		if (!retActor)
		{
			return lockedTarget;
		}
	}
	return retActor;
}

void AServant::EstablishLock()
{
	if (isClimbing) return;
	if (isNobleTracing) return;
	ResetCameraToStandard();

	lockCooldown = 0;
	if (lockedOn)
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;

		cameraBoom->SetRelativeRotation(FRotator(-20, 0, 0));
		cameraBoom->SetRelativeLocation(FVector::ZeroVector);

		if (UIWidget) UIWidget->ShowLocOnMark(false);
		animInstance->TraceMode(false);
		lockedOn = false;
		lockedTarget = nullptr;
	}
	else
	{
		lockedTarget = Cast<AServant>(FindClosestActorInDirectionByAngle(true, true));
		if (!lockedTarget) return;

		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;

		cameraBoom->SetRelativeLocation(FVector(0, 100, 0));
		cameraBoom->SetRelativeRotation(FRotator(-20, 0, 0));

		if (UIWidget) UIWidget->ShowLocOnMark(true);
		animInstance->TraceMode(true);
		lockedOn = true;


		OrientToLockedTarget(GetWorld()->GetDeltaSeconds());
		sprintPressCount = 2;
	}
}

void AServant::OrientToLockedTarget(float DeltaTime)
{
	if (lockCooldown > 0) lockCooldown -= DeltaTime;
	if (lockedTarget)
	{
		if (UIWidget) UIWidget->UpdateLocPosition(lockedTarget->GetActorLocation());

		FVector dirTo = (lockedTarget->GetActorLocation() - GetActorLocation()) * FVector(1, 1, 0);
		dirTo.Normalize();

		GetController()->SetControlRotation(FMath::RInterpTo(GetControlRotation(), FRotator(GetControlRotation().Pitch, dirTo.ToOrientationRotator().Yaw, 
			GetControlRotation().Roll), DeltaTime, 3));

		if (aimRestricted) return;
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), FRotator(0, dirTo.ToOrientationRotator().Yaw, 0), DeltaTime, 1));
	}
	else
	{
		EstablishLock();
	}
}

void AServant::ReportLockedTargetDestroyed()
{
	if (lockedOn)
	{
		lockedTarget = Cast<AServant>(FindClosestActor());
		if (!lockedTarget)
		{
			EstablishLock();
		}
		else
		{
			OrientToLockedTarget(GetWorld()->GetDeltaSeconds());
		}
	}
}

#pragma endregion

#pragma region health

void AServant::DoDamage(float dmgIn)
{
	if (invincible || noHit) return;

	if (dmgIn > 1) dmgIn = dmgIn - 1;
	if (health < 0 || dmgIn <= 0) return;

	health -= dmgIn;

	if (UIWidget) UIWidget->UpdateHealth(health);

	if (health < 0)
	{
		if (!isPlayer) CharacterDestroyByDamage();
		else
		{
			waitTime = 5.f;
			cameraBoom->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("spine_01"));
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
			GetMesh()->SetSimulatePhysics(true);
			GetMesh()->SetPhysicsLinearVelocity(-1 * GetActorForwardVector() * 10000 * FMath::FRandRange(.5, 1.256));
		}
	}
}

void AServant::DoTakeHit(const FString &specialEffect)
{
	if (noHit) return;
	animInstance->DoDamageEffect(specialEffect);
}

void AServant::CharacterDestroyByDamage()
{
	inSelfDestroySequence = true;

	if (personalWeapon) personalWeapon->DoWeaponDropAndRemove();
	if (aiManager && !isPlayer) aiManager->ReportEnemyDestroyed(this);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetPhysicsLinearVelocity(-1 * GetActorForwardVector() * 10000 * FMath::FRandRange(.5, 1.256));
}

void AServant::HandleSelfDestroySequence(float DeltaTime)
{
	timeToSelfDestroy -= DeltaTime;

	if (timeToSelfDestroy < 0)
	{
		Destroy();
	}
}

#pragma endregion

#pragma region combat

void AServant::ParseAttackCommand()
{
	animInstance->AttackCommand(false, 1);
}

void AServant::ParseHoldAttackCommand()
{
	if (specPercentage < .25 && personalWeapon) return;
	animInstance->HoldAttackCommand();
}

void AServant::StartAttackEvent(int attackCode, float inComboDepth, float addScanTime)
{
	if (!lockedOn)
	{
		if (!FMath::IsNearlyZero(GetInputAxisValue("MoveForward")) || !FMath::IsNearlyZero(GetInputAxisValue("MoveRight")))
		{
			float forwardInput = GetInputAxisValue("MoveForward");
			float rightInput = GetInputAxisValue("MoveRight");
			FVector forwardComp = UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0)) * (forwardInput);
			FVector rightComp = UKismetMathLibrary::GetRightVector(FRotator(0, GetControlRotation().Yaw, 0)) * (rightInput);
			SetActorRotation((forwardComp + rightComp).ToOrientationRotator());
		}
	}

	if (isSprinting)
	{
		sprintPressCount = 2;
		DisengageSprintMode();
	}

	currentAttackSpecialEffect = "None";
	if (!personalWeapon)
	{
		if (attackCode == 5 || attackCode == 4) currentAttackSpecialEffect = "KnockBack";
		comboMult = inComboDepth;
		StartUnarmedAttack();
	}
	else
	{ 
		personalWeapon->StartAttackHandler(attackCode, inComboDepth, addScanTime);
	}
}

void AServant::EndAttackEvent(int attackCode)
{
	if (!personalWeapon)
	{
		EndUnarmedAttack();
	}
	else
	{
		personalWeapon->EndAttackHandler(attackCode);
	}
}

void AServant::StartWeaponDangerous()
{
	if (!personalWeapon)
	{
		rHandOverlap->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		lHandOverlap->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		rFootOverlap->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		rHandOverlap->SetGenerateOverlapEvents(true);
		lHandOverlap->SetGenerateOverlapEvents(true);
		rFootOverlap->SetGenerateOverlapEvents(true);
	}
	else
	{
		personalWeapon->StartWeaponDangerous();
	}
}

void AServant::EndWeaponDangerous()
{
	if (!personalWeapon)
	{
		rHandOverlap->SetGenerateOverlapEvents(false);
		lHandOverlap->SetGenerateOverlapEvents(false);
		rFootOverlap->SetGenerateOverlapEvents(false);

		rHandOverlap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		lHandOverlap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		rFootOverlap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{

		personalWeapon->EndWeaponDangerous();
	}
}

void AServant::StartUnarmedAttack()
{
	EmptyAttackAffectedActors();
}

void AServant::EndUnarmedAttack()
{

}

void AServant::ProcessEnemyHitEvent(AActor* otherActor)
{
	if (!otherActor) return;
	DoHitOther(otherActor);

	float damage = .05 * FMath::FRandRange(.1, 2) * comboMult;
	Cast<AServant>(otherActor)->DoTakeHit(currentAttackSpecialEffect);
	Cast<AServant>(otherActor)->DoDamage(damage);

	//IMyTypes::Debug_Print(damage, 10, 10);
}

bool AServant::IsActorAffectedByCurrentAttack(AActor* otherActor)
{
	if (otherActor)
	{
		return thisMoveAffectedActors.Contains<AActor*>(otherActor);
	}
	else return false;
}

void AServant::ResetAffectedActors()
{
	EmptyAttackAffectedActors();
}

void AServant::AddAttackAffectedActor(AActor* otherActor)
{
	if (otherActor)
	{
		thisMoveAffectedActors.Add(otherActor);
	}
}

void AServant::EmptyAttackAffectedActors()
{
	thisMoveAffectedActors.Empty();
}

void AServant::OnOverlapBeginUnarmed(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == personalWeapon || OtherActor == this) return;

	if (OtherActor->IsA(APawn::StaticClass()))
	{
		if (!IsActorAffectedByCurrentAttack(OtherActor))
		{
			AddAttackAffectedActor(OtherActor);
			ProcessEnemyHitEvent(OtherActor);
		}
	}
}

void AServant::HandleHitStop(float DeltaTime)
{
	hitStopRemainTime -= DeltaTime;

	if (hitStopRemainTime < 0)
	{
		hitStopActive = false;

		if (animInstance->IsAnyMontagePlaying() && isPlayer)
		{
			UAnimMontage* currentMontage = animInstance->GetCurrentActiveMontage();
			if (currentMontage)
			{
				currentMontage->RateScale = 1;
			}
		}
	}
}

void AServant::DoHitOther(AActor* otherActor)
{
	if (!otherActor) return;
	AddAttackAffectedActor(otherActor);
}

void AServant::DoPlayFFB(UForceFeedbackEffect* effectToPlay)
{
	if (!effectToPlay) return;
	AController* cont = GetController();
	if (cont)
	{
		if (cont->IsPlayerController())
		{
			APlayerController* myPCont = Cast<APlayerController>(cont);
			if (myPCont)
			{
				myPCont->ClientPlayForceFeedback(effectToPlay, FForceFeedbackParameters());
			}
		}
	}
}

void AServant::ReportEnemyDestroyed(bool servant)
{
	if (servant) enemiesDefeated += 25;
	else enemiesDefeated++;
	
	
	if (UIWidget) UIWidget->EnemyDefeat(enemiesDefeated, enemiesToDefeat);

	if (enemiesDefeated >= enemiesToDefeat)
	{
		waitTime = 5.f;
	}
}

void AServant::AddSpecPercent(float specAddAmount)
{
	specPercentage += specAddAmount;

	if (specPercentage > 1) specPercentage = 1;

	if (UIWidget) UIWidget->UpdateSpecPercent(specPercentage);
}

void AServant::AddCrestPercent(float crestAddAmount)
{
	crestPercentage += crestAddAmount;

	if (crestPercentage > 1) crestPercentage = 1;

	if (UIWidget) UIWidget->UpdateCrestPercent(crestPercentage);
}

void AServant::ReportAttackCommandSuccess(int code)
{
	if (code == HOLD_CODE)
	{
		specPercentage -= .25;
		if (specPercentage < 0) specPercentage = 0;

		if (UIWidget) UIWidget->UpdateSpecPercent(specPercentage);
	}
	else if (code < 0)
	{
		inputBuffer.Add(code * -1);
	}

}

#pragma endregion

#pragma region combat overlaps

void AServant::OnOverlapBeginDamageVolume(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(AServant::StaticClass()) && OtherActor != this)
	{
		int a = activeAEOOverlaps.Find(Cast<USphereComponent>(OverlappedComp));
		Cast<AServant>(OtherActor)->DoDamage(activeAOEDamages[a]);
		Cast<AServant>(OtherActor)->DoTakeHit("KnockBack");
	}
}

void AServant::DoAOEImmediate(float size, FVector initLocation, bool attachToThis, float aoeExistTime, float damageToDo)
{
	USphereComponent* myComp = Cast<USphereComponent>(NewObject<USphereComponent>(this, USphereComponent::StaticClass()));
	if (!myComp) return;

	activeAEOOverlaps.Add(myComp);
	activeAOETimes.Add(aoeExistTime);
	activeAOEDamages.Add(damageToDo);

	myComp->RegisterComponent();

	if (attachToThis)
	{
		myComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		myComp->SetRelativeLocation(initLocation);
	}
	else
	{
		myComp->SetWorldLocation(initLocation);
	}


	myComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	myComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	myComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	myComp->OnComponentBeginOverlap.AddDynamic(this, &AServant::OnOverlapBeginDamageVolume);

	myComp->SetSphereRadius(size, true);


}

void AServant::HandleAOEDespawn(float DeltaTime)
{
	//IMyTypes::Debug_Print("Num overlaps: " + FString::FromInt(activeAEOOverlaps.Num()) + "," + FString::FromInt(activeAOETimes.Num()) + "," + FString::FromInt(activeAOEDamages.Num()), 10, 1);

	if (activeAEOOverlaps.Num() == 0)
	{
		activeAOETimes.Empty();
		activeAOEDamages.Empty();
	}

	for (int i = activeAEOOverlaps.Num() - 1; i >= 0; i--)
	{
		activeAOETimes[i] -= DeltaTime;
		//IMyTypes::Debug_Print("remain: " + FString::SanitizeFloat(activeAOETimes[i]), 10, 2);

		if (activeAOETimes[i] < 0)
		{
			USphereComponent* p = activeAEOOverlaps[i];
			activeAEOOverlaps.RemoveAt(i);
			activeAOETimes.RemoveAt(i);
			activeAOEDamages.RemoveAt(i);
			p->DestroyComponent();
		}
	}

}

#pragma endregion

#pragma region weapons

void AServant::TraceOn()
{
	int numTraced = done_tracedWeapons.Num();
	if (numTraced < 1) return;

	ResetCameraToStandard();

	DestroyTempDisplayWeapon();

	if (personalWeapon)
	{
		if (personalWeapon->nobleWeapon) return;
		animInstance->TraceOn_FirstDiscard();
	}
	else
	{
		animInstance->TraceOn_Normal();
	}

}

void TraceOn_Init_Failed()
{

}

bool AServant::TraceOn_Personal()
{
	if (done_tracedWeapons.Num() < 1) return false;

	FActorSpawnParameters myP = FActorSpawnParameters();
	myP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UClass* classToSpawn = (UClass*)done_tracedWeapons[traceWeaponIndex];
	if (!classToSpawn) return false;

	personalWeapon = Cast<AWeapon>(GetWorld()->SpawnActor<AWeapon>(classToSpawn, FVector::ZeroVector, FRotator(0, 0, 0), myP));
	InitSpawnedWeapon(true, personalWeapon);
	RemoveFromTracedWeapons(traceWeaponIndex);
	return true;
}

bool AServant::TraceOn_Launch()
{
	int numTraced = done_tracedWeapons.Num();

	if (done_tracedWeapons.Num() > 0)
	{
		if (currentSpawnedWeapons < maxSpawnedWeapons)
		{
			FActorSpawnParameters myP = FActorSpawnParameters();
			myP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			FVector spawnLoc = GetScanningWeaponSpawnLocation(currentSpawnedWeapons);

			UClass* classToSpawn = (UClass*)done_tracedWeapons[numTraced - 1];
			RemoveFromTracedWeapons(numTraced - 1);
			AWeapon* spawnedWeapon = Cast<AWeapon>(GetWorld()->SpawnActor<AWeapon>(classToSpawn, spawnLoc, FRotator(0, 0, 0), myP));

			InitSpawnedWeapon(false, spawnedWeapon);
			currentSpawnedWeapons += 1;
		}
	}
	else if (personalWeapon)
	{
		if (personalWeapon->nobleWeapon) return true;
		animInstance->TraceOn_DiscardOnly();
	}

	return false;
}

bool AServant::TraceOn_Discard()
{
	if (!personalWeapon) return false;
	AActor* trgt = (lockedTarget ? lockedTarget : FindClosestActor());

	personalWeapon->InitProjectileMode(FVector::UpVector * 100, this, trgt, .6);

	personalWeapon = nullptr;
	return true;
}

void AServant::InitSpawnedWeapon(bool personal, AWeapon* spawnedWeapon)
{
	if (!spawnedWeapon) return;

	if (personal)
	{
		spawnedWeapon->canScan = false;
		spawnedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "right_hand_socket");
		spawnedWeapon->UseWeaponAsActorOrientation(GetMesh()->GetSocketLocation("weapon_point_socket") - GetMesh()->GetSocketLocation("right_hand_socket"));
		spawnedWeapon->InitHeldMode(this);
		animInstance->SetAttackMontage(spawnedWeapon->weaponAttackMontage);
		animInstance->SetMovementAnimations(spawnedWeapon->idleAnim, spawnedWeapon->movementAnim, spawnedWeapon->traceOnMontage);
	}
	else
	{
		spawnedWeapon->canScan = false;
		spawnedWeapons.Emplace(spawnedWeapon);
		spawnedWeapon->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		spawnedWeapon->SetActorLocation(GetScanningWeaponSpawnLocation(currentSpawnedWeapons));
		spawnedWeapon->UseWeaponAsActorOrientation(GetCapsuleComponent()->GetForwardVector());
		spawnedWeapon->SetJitterSelf(true);
	}

	spawnedWeapon->canScan = false;
	spawnedWeapon->SetMeshHidden(true);
	spawnedWeapon->KickOffSpawnSequence();
}

void AServant::TrySpawnWeapon()
{
	int numTraced = done_tracedWeapons.Num();
	if (done_tracedWeapons.Num() < 1) return;

	if (currentGameMode == EGameMode::EGM_Trace && currentSpawnedWeapons < maxSpawnedWeapons)
	{
		FActorSpawnParameters myP = FActorSpawnParameters();
		myP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (numTraced - 1 == traceWeaponIndex)
		{
			if (numTraced == 1)
			{
				traceWeaponIndex = 0;
				DestroyTempDisplayWeapon();
			}
			else
			{
				traceWeaponIndex--;
			}
		}

		UClass* classToSpawn = (UClass*)done_tracedWeapons[numTraced - 1];
		RemoveFromTracedWeapons(numTraced - 1);
		AWeapon* spawnedWeapon = Cast<AWeapon>(GetWorld()->SpawnActor<AWeapon>(classToSpawn, FVector::ZeroVector, FRotator(0, 0, 0), myP));

		InitSpawnedWeapon(false, spawnedWeapon);
		currentSpawnedWeapons += 1;
	}
}

void AServant::TryShootWeapon()
{
	DestroyTempDisplayWeapon();
	if (spawnedWeapons.Num() > 0)
	{
		AActor* trgt = (lockedTarget ? lockedTarget : FindClosestActor());
		for (int i = spawnedWeapons.Num()-1; i >= 0; i--)
		{
			AWeapon* weaponToShoot = spawnedWeapons[i];

			if (weaponToShoot)
			{
				weaponToShoot->InitProjectileMode(FVector::ZeroVector, this, trgt, (float(i) * .25));
				spawnedWeapons.Remove(weaponToShoot);
			}
		}

		currentSpawnedWeapons = spawnedWeapons.Num();
		//IMyTypes::Debug_Print(currentSpawnedWeapons, 10);

	}
	else if (done_tracedWeapons.Num() > 0)
	{
		TrySpawnWeapon();
	}
}

void AServant::DestroyPersonalWeapon()
{
	if (personalWeapon)
	{
		personalWeapon->Destroy();
		personalWeapon = nullptr;

		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
}

void AServant::SelectTracedWeapon()
{
	if (traceWeaponIndex < 0 || done_tracedWeapons.Num() < 2 || traceWeaponIndex > done_tracedWeapons.Num()) return;

	SpawnTempDisplayWeapon();
}

void AServant::SpawnTempDisplayWeapon()
{
	if (tempDisplayWeapon) DestroyTempDisplayWeapon();

	//IMyTypes::Debug_Print("Spawn debug weapon", 10);

	FActorSpawnParameters myP = FActorSpawnParameters();
	myP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	UClass* classToSpawn = (UClass*)done_tracedWeapons[traceWeaponIndex];
	tempDisplayWeapon = Cast<AWeapon>(GetWorld()->SpawnActor<AWeapon>(classToSpawn, FVector::ZeroVector, FRotator(0, 0, 0), myP));

	if (tempDisplayWeapon)
	{
		tempDisplayWeapon->canScan = false;
		tempDisplayWeapon->GetWeaponMesh()->SetHiddenInGame(true);
		tempDisplayWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "preview_socket");
		tempDisplayWeapon->UseWeaponAsActorOrientation(GetMesh()->GetSocketLocation("preview_socket_point") - GetMesh()->GetSocketLocation("preview_socket"));
		tempDisplayWeapon->StartPreviewParticle();
		displayingTempWeapon = true;
		tempDisplayWeaponTimeout = tempDisplayWeaponTimeoutMax;
	}

}

void AServant::TimeoutTempDisplayWeapon(float DeltaTime)
{
	tempDisplayWeaponTimeout -= DeltaTime;

	if (tempDisplayWeaponTimeout < 0)
	{
		DestroyTempDisplayWeapon();
	}
}

void AServant::DestroyTempDisplayWeapon()
{
	if (tempDisplayWeapon)
	{
		//IMyTypes::Debug_Print("Destory debug", 10);
		displayingTempWeapon = false;
		tempDisplayWeapon->DestroyPreviewParticle();
		tempDisplayWeapon->Destroy();
		tempDisplayWeapon = nullptr;
	}
}

void AServant::ReportDestroyedWeapon()
{
	animInstance->WeaponDestroyed();
	personalWeapon = nullptr;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void AServant::AttachWeaponToSocket(const FString &inName)
{
	if (!personalWeapon) return;

	personalWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName(inName));
	personalWeapon->UseWeaponAsActorOrientation(GetMesh()->GetSocketLocation("weapon_point_socket") - GetMesh()->GetSocketLocation("right_hand_socket"));
}

#pragma region noble

void AServant::TraceOn_Noble()
{
	if (nobleCache.Num() < 1) return;

	ResetCameraToStandard();

	FActorSpawnParameters myP = FActorSpawnParameters();
	myP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UClass* classToSpawn = (UClass*)nobleCache[FMath::RandRange(0, nobleCache.Num() - 1)];
	if (!classToSpawn) return;

	personalWeapon = Cast<AWeapon>(GetWorld()->SpawnActor<AWeapon>(classToSpawn, FVector::ZeroVector, FRotator(0, 0, 0), myP));

	personalWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "left_hand_socket");
	personalWeapon->UseWeaponAsActorOrientation(GetMesh()->GetSocketLocation("weapon_point_socket2") - GetMesh()->GetSocketLocation("left_hand_socket"));
	personalWeapon->InitHeldMode(this);
	animInstance->SetAttackMontage(personalWeapon->weaponAttackMontage);
	animInstance->SetMovementAnimations(personalWeapon->idleAnim, personalWeapon->movementAnim, personalWeapon->traceOnMontage);

	personalWeapon->canScan = false;
	personalWeapon->SetMeshHidden(true);
	personalWeapon->KickOffSpawnSequence();

	crestPercentage = 0;
	UIWidget->UpdateCrestPercent(crestPercentage);
}

void AServant::ReportNobleWeaponUseOver()
{
	if (!launchWeaponMode || !personalWeapon)
	{
		if (personalWeapon->nobleWeapon)
		{
			HandleNobleBreakMode();
		}
	}
}

void AServant::InitLaunchWeaponMode()
{
	if (!personalWeapon) return;
	maxBreakBowTime = 6;

	TSoftClassPtr<AWeapon> myBowClass = TSoftClassPtr<AWeapon>(FSoftObjectPath(TEXT("/Game/Weapons/Noble/Noble_Bow.Noble_Bow_C")));
	UClass* myPtr = myBowClass.LoadSynchronous();

	FActorSpawnParameters myP = FActorSpawnParameters();
	myP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (breakBow) DestroyBreakBow();

	breakBow = Cast<AWeapon>(GetWorld()->SpawnActor<AWeapon>(myPtr, FVector::ZeroVector, FRotator(0, 0, 0), myP));
	breakBow->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "break_bow_socket");
	breakBow->SetActorScale3D(FVector(1, .5, 1));
	//breakBow->UseWeaponAsActorOrientation(GetMesh()->GetSocketLocation("weapon_point_socket2") - GetMesh()->GetSocketLocation("break_bow_socket"));
	breakBow->OrientSelfToWeapon(2);
	breakBow->canScan = false;
	breakBow->SetMeshHidden(true);
	breakBow->KickOffSpawnSequence();
	breakBow->GetWeaponMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	breakBow->GetWeaponMesh()->SetAnimInstanceClass(breakBowAnimClass);
	breakBowDestroy = false;

	launchWeaponMode = true;
	personalWeapon->OrientSelfToWeapon(3);
	//personalWeapon->UseWeaponAsActorOrientation(GetMesh()->GetSocketLocation("right_hand_socket") - breakBow->GetWeaponMesh()->GetSocketLocation("bow"));

	FVector cacheSize = traceVolumeMaxExtent;
	traceVolumeMaxExtent = FVector(4000, 4000, 4000);
	InitTraceVolume();
	traceVolumeMaxExtent = cacheSize;

	if (currentCameraSetting == ECM_Standard)
	{
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}

}

void AServant::ProcessLaunchWeaponMode(float DeltaTime)
{
	maxBreakBowTime -= DeltaTime;

	if (maxBreakBowTime < 0)
	{
		HandleNobleBreakMode();
		maxBreakBowTime = 6;
		launchWeaponMode = false;
	}
}

void AServant::LaunchNobleWeapon()
{
	if (!personalWeapon) return;
	personalWeapon->InitProjectileImmediate(this, (lockedTarget ? lockedTarget : FindClosestActor()));

	launchWeaponMode = false;

	if (lockedOn) EstablishLock();

	breakBowDestroy = true;
	breakBowWaitDestroy = 2;


	if (currentCameraSetting == ECM_Standard)
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}

	animInstance->WeaponDestroyed();
	personalWeapon = nullptr;

	EndMovementControl();
}

void AServant::HandleNobleBreakMode()
{
	if (launchWeaponMode)
	{
		animInstance->TraceOn_Noble_Break_Launch();
		if (breakBow)
		{
			Cast<UServantAnimInstance>(breakBow->GetWeaponMesh()->GetAnimInstance())->TraceOn_Noble_Break_Launch();
		}
		return;
	}

	if (personalWeapon)
	{
		if (personalWeapon->nobleWeapon)
		{
			animInstance->TraceOn_Noble_Break();
			return;
		}
	}
}

void AServant::ProcessBreakBowDestroy(float DeltaTime)
{
	breakBowWaitDestroy -= DeltaTime;

	if (breakBowWaitDestroy < 0)
	{
		DestroyBreakBow();
	}
}

void AServant::DestroyBreakBow()
{
	if (breakBow)
	{
		Cast<UServantAnimInstance>(breakBow->GetWeaponMesh()->GetAnimInstance())->TraceOn_DiscardOnly();
		breakBow->SetActorHiddenInGame(true);
		breakBow->DoWeaponDestroyAndRemove();
	}
	breakBowDestroy = false;
	breakBow = nullptr;
}

#pragma endregion

#pragma endregion

#pragma region trace

#pragma region init and cancel

void AServant::InitLineTrace()
{
	DestroyTempDisplayWeapon();
	isLineTracing = true;
	UpdateLineTrace();
	animInstance->Do_TraceLine(true);
	traceBeam->Activate();
}

void AServant::CancelLineTrace()
{
	isLineTracing = false;
	animInstance->Do_TraceLine(false);
	traceBeam->DeactivateImmediate();

	for (auto inTrace : in_tracingWeapons)
	{
		if (!inTrace->isScannedByBox)
		{
			inTrace->lastScanMiss = true;
		}
	}
}

void AServant::EnsureLineTraceCancel()
{
	isLineTracing = false;
	if (traceBeam->IsActive()) traceBeam->DeactivateImmediate();

	for (auto inTrace : in_tracingWeapons)
	{
		if (!inTrace->isScannedByBox)
		{
			inTrace->lastScanMiss = true;
		}
	}
}

void AServant::InitTraceVolume()
{
	traceVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	isVolumeTracing = true;

	traceVolume->SetBoxExtent(traceVolumeMaxExtent, true);
}

void AServant::StartCombatVolumeTrace()
{
	if (isVolumeTracing) return;
	InitTraceVolume();
}

void AServant::CancelTraceVolume()
{
	traceVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	isVolumeTracing = false;
	traceVolumeTime = traceVolumeTimeMax;

	traceVolume->SetBoxExtent(FVector(1), true);
}

#pragma endregion

#pragma region trace update

void AServant::UpdateTracingWeapons()
{

}

void AServant::UpdateLineTrace()
{
	FHitResult probeTrace = FHitResult();
	FVector handLoc = GetMesh()->GetSocketLocation(FName("right_hand_socket"));
	FVector cameraDir = camera->GetForwardVector() * 3000;

	traceBeam->SetVectorParameter("Beam Start", handLoc);
	traceBeam->SetVectorParameter("Beam End", handLoc + cameraDir);

	FCollisionObjectQueryParams queryParams = FCollisionObjectQueryParams();
	queryParams.AddObjectTypesToQuery(IMyTypes::Weapon_Srch);
	TArray<FHitResult> hitResultArray;

	GetWorld()->LineTraceMultiByObjectType(hitResultArray, handLoc, handLoc + cameraDir, queryParams);

	for (auto hit : hitResultArray)
	{
		TryTraceWeapon(Cast<AWeapon>(hit.GetActor()));
	}

	for (auto inTrace : in_tracingWeapons)
	{
		bool found = false;
		for (auto hit : hitResultArray)
		{
			if (hit.IsValidBlockingHit())
			{
				if (inTrace == hit.GetActor())
				{
					found = true;
					break;
				}
			}
		}
		if (!found && !inTrace->isScannedByBox)
		{
			inTrace->lastScanMiss = true;
		}
	}

	animInstance->SetAimAngle(camera->GetRelativeTransform().Rotator().Pitch);

}

bool AServant::TryTraceWeapon(AWeapon* weaponToScan)
{
	if (!weaponToScan) return false;
	if (!weaponToScan->canScan) return false;

	if (!in_tracingWeapons.Contains<AWeapon*>(weaponToScan))
	{
		if (in_tracingWeapons.Num() < maxTracingWeapons)
		{
			weaponToScan->SetScanner(this);
			AddToTracingWeapons(weaponToScan);
		}
		else return false;
	}
	if (!weaponToScan->isScannedByBox) weaponToScan->AddScanTime(GetWorld()->GetDeltaSeconds());
	weaponToScan->lastScanMiss = false;
	return true;
}

void AServant::AddToTracingWeapons(AWeapon* inWep)
{
	if (!inWep) return;
	int numWeaponsTracing = in_tracingWeapons.Num();

	if (numWeaponsTracing < maxTracingWeapons)
	{
		in_tracingWeapons.Add(inWep);
		UIWidget->AddWeapon(inWep);
	}
}

void AServant::RemoveFromTracingWeapons(AWeapon* inWep)
{
	if (!inWep) return;

	in_tracingWeapons.Remove(inWep);
	in_tracingWeapons.Shrink();
	UIWidget->RemoveWeapon(inWep);
}

void AServant::ReportFullTrace(AWeapon* inWep)
{
	if (!inWep) return;
	if (!in_tracingWeapons.Contains<AWeapon*>(inWep)) return;

	if (health < 1)	health += .05;
	if (health > 1) health = 1;

	if (UIWidget) UIWidget->UpdateHealth(health);

	if (inWep->nobleWeapon)
	{
		if (!nobleCache.Contains((TSubclassOf<AWeapon>*)inWep->GetClass()))
		{
			nobleCache.Add((TSubclassOf<AWeapon>*)inWep->GetClass());

			if (myGameInstance)
			{
				myGameInstance->nobleCache = nobleCache;
			}
		}
	}
	else
	{
		if (AddToTracedWeapons((TSubclassOf<AWeapon>*)inWep->GetClass()))
		{
			RemoveFromTracingWeapons(inWep);
		}
	}

}

void AServant::ReportTraceFailed(AWeapon* inWep)
{
	if (in_tracingWeapons.Contains<AWeapon*>(inWep))
	{
		RemoveFromTracingWeapons(inWep);
	}
}

FVector AServant::GetScanningWeaponSpawnLocation(float numSpawned)
{
	float col = FMath::FloorToFloat(numSpawned / 5) + 1;
	float row = FMath::Fmod(numSpawned, 5.f);

	FVector acLoc = GetCapsuleComponent()->GetComponentLocation();
	FVector acUp = GetCapsuleComponent()->GetUpVector();
	FVector acFwd = GetCapsuleComponent()->GetForwardVector();

	FVector init = acUp;
	FVector axis = acFwd;
	FVector result = init.RotateAngleAxis(-60 + (row) * 30, axis);
	result.Normalize();

	result = result * col * 100;
	return acLoc + result;
}

#pragma endregion

#pragma region trace box

void AServant::FloatTraceVolume(float DeltaSeconds)
{
	traceVolumeTime -= DeltaSeconds;

	if (traceVolumeTime < 0)
	{
		CancelTraceVolume();
		return;
	}


	for (int i = 0; i < in_tracingWeapons.Num(); i++)
	{
		AWeapon* inScanWep = in_tracingWeapons[i];
		if (inScanWep->isScannedByBox)
		{
			inScanWep->AddScanTime(DeltaSeconds);
		}
	}

}

void AServant::OnOverlapBeginTrace(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (OtherActor->IsA(AWeapon::StaticClass()))
	{
		AWeapon* inWep = Cast<AWeapon>(OtherActor);
		if (!inWep) return;
		if (TryTraceWeapon(inWep))
		{
			inWep->isScannedByBox = true;
		}
	}
}

void AServant::OnOverlapEndTrace(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (otherActor->IsA(AWeapon::StaticClass()))
	{
		AWeapon* inWep = Cast<AWeapon>(otherActor);
		if (!inWep) return;
		inWep->isScannedByBox = false;
		inWep->lastScanMiss = true;
	}
}

#pragma endregion

#pragma region fully traced

bool AServant::AddToTracedWeapons(TSubclassOf<AWeapon>* tracedWep)
{
	if (!tracedWep) return false;
	if (done_tracedWeapons.Num() < maxTracedWeapons)
	{
		done_tracedWeapons.Add(tracedWep);

		availableWeaponsSystem->SetNiagaraVariableInt("Count", done_tracedWeapons.Num());
		availableWeaponsSystem->DeactivateImmediate();
		availableWeaponsSystem->Activate();


		return true;
	}
	return false;
}

void AServant::RemoveFromTracedWeapons(int index)
{
	// check if temp weapon is being displayed TODO
	if (done_tracedWeapons.Num() > 0)
	{
		if (index >= 0 && index < done_tracedWeapons.Num())
		{
			if (traceWeaponIndex == index)
			{
				if (traceWeaponIndex > 0)
				{
					traceWeaponIndex--;
				}
			}
			done_tracedWeapons.RemoveAt(index);
			done_tracedWeapons.Shrink();

			availableWeaponsSystem->SetNiagaraVariableInt("Count", done_tracedWeapons.Num());
			availableWeaponsSystem->DeactivateImmediate();
			availableWeaponsSystem->Activate();

			if (done_tracedWeapons.Num() == 0)
			{
				availableWeaponsSystem->DeactivateImmediate();
			}
		}
	}
}

#pragma endregion

#pragma endregion
