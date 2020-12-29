// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Servant.generated.h"

UCLASS()
class MOVEMENT_API AServant : public ACharacter
{
	GENERATED_BODY()

private:

	typedef enum EGameMode
	{
		EGM_Standard = 0,
		EGM_Trace = 1,
		EGM_Cinematic = 2
	};

	typedef enum ECameraMode
	{
		ECM_Standard = 0,
		ECM_Trace = 1,
		ECM_Cinematic = 2
	};

	UPROPERTY(EditAnywhere)
	class UCameraComponent* camera;
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* cameraBoom;
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* targetingCameraBoom;

protected:

	bool isPlayer = false;
	class UServantAnimInstance* animInstance;

#pragma region health

	float health = 1;
	virtual void CharacterDestroyByDamage();

	bool inSelfDestroySequence = false;
	float timeToSelfDestroy = 10.f;
	void HandleSelfDestroySequence(float DeltaTime);

#pragma endregion

#pragma region UI

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> UIWidgetBase;
	class UServantUI* UIWidget;

#pragma endregion

#pragma region world

	float waitTime = 0;
	void ProcessWaitTime(float DeltaTime);

	class UServantGameInstance* myGameInstance;

#pragma endregion

#pragma region movement and aim

	float dodgeTime = 0;
	class UCurveVector* dodgeCurve;

	bool lastResult = false;
	void ReportMovementInput();
	UFUNCTION(BlueprintCallable)
	void SetMovementRestricted(bool isMoveRestrict, bool isAimRestrict, bool isDodgeRestrict);

	bool aimRestricted = false;
	bool movementRestricted = false;
	int moveRestrictCounters = 0;
	UPROPERTY(EditAnywhere)
	float nonSprintSpeed = 10.f;
	UPROPERTY(EditAnywhere)
	float sprintSpeed = 5.f;
	int sprintPressCount = 0;
	void EngageSprintMode();
	void DisengageSprintMode();
	bool isSprinting = false;

	void MoveUp(float axisval);
	void MoveRight(float axisval);
	void AimUp(float axisval);
	void AimRight(float axisval);
	void Jump();

	FVector commitDodgeDir = FVector::ZeroVector;
	bool canDodge = true;
	bool isDodging = false;
	UFUNCTION(BlueprintCallable)
	void StartDodge();
	void ProcessDodge(float DeltaTime);
	UFUNCTION(BlueprintCallable)
	void EndDodge();

	int forceMovementCounter = 0;
	class UCurveVector* forceMovementCurve;
	bool forceMovementControl = false;
	float timeInMovement = 0;
	void ProcessMovementControl(float DeltaTime);
	UFUNCTION(BlueprintCallable)
	void StartMovementControl(class UCurveVector* inCurve, bool canIgnorePawn);
	UFUNCTION(BlueprintCallable)
	void EndMovementControl();

#pragma endregion

#pragma region camera

	void ResetCameraToStandard();

	const int STANDARD_CAMERA = 0;
	const int TARGET_CAMERA = 1;
	ECameraMode currentCameraSetting = ECameraMode::ECM_Standard;
	void SetCameraMode(ECameraMode newMode);
	void ToggleCamera();
	void ResetCamera();

	bool isInterpCameraAim = false;
	bool isInterpCameraPos = false;
	bool isInterpCameraRot = false;
	float desiredCameraPitch = 0.f;
	FVector desiredCameraPos = FVector::ZeroVector;
	FRotator desiredCameraRot = FRotator::ZeroRotator;
	void InterpCameraAim(float DeltaTime);
	void InterpCameraPos(float DeltaTime);
	void InterpCameraRot(float DeltaTIme);

	float timeInNobleCinematic = 0;
	UFUNCTION(BlueprintCallable)
	void InterpCameraNoble(float DeltaTime);
	void ProcessNobleCinematic(float DeltaTime);
	UFUNCTION(BlueprintCallable)
	void StartNobleCinematic();
	UFUNCTION(BlueprintCallable)
	void EndNobleCinematic();

#pragma endregion

#pragma region lock

	bool lockedOn = false;

	float lockCooldown = 0;
	AServant* lockedTarget;
	AActor* FindClosestActor();
	AActor* FindClosestActorInDirection(bool left);
	AActor* FindClosestActorInDirectionByAngle(bool left, bool either = false);
	void EstablishLock();

	void OrientToLockedTarget(float DeltaTime);

#pragma endregion

#pragma region input

	void ParsePrimaryInput();
	void ParseHoldInput();
	void ParseSecondaryInput();
	void ParseTertiaryInput();
	void ParseEndPrimaryInput();
	void ParseSpecialInput();
	void ParseShootInput();
	void ParseDodgeInput();

	UPROPERTY(EditAnywhere)
	float holdInputRegisterTime = .6f;
	float currentAttackInputHoldTime = 0;
	bool isHoldingAttackInput = false;
	void ProcessHoldInput(float DeltaTime);

#pragma endregion

#pragma region combat

	TArray<int> inputBuffer;
	float timeToConsumeBuffer = .5;
	void ProcessBufferAutoConsume(float DeltaTime);

	int HOLD_CODE = 9;

	int enemiesToDefeat = 100;
	int enemiesDefeated = 0;

	FString currentAttackSpecialEffect = "None";

	float armor = 0;
	float comboMult = 1;
	bool hitStopActive = false;
	float hitStopRemainTime = .05;
	float hitStopRemainTimeMax = .05;
	void HandleHitStop(float DeltaTime);

	TArray<AActor*> thisMoveAffectedActors;

	UFUNCTION(BlueprintCallable)
	void SetIFrame(bool inInvincible);

	bool invincible = false;
	bool noHit = false;

	void ParseAttackCommand();
	void ParseHoldAttackCommand();
	void ParseNobleTraceCommand();

	float crestPercentage = 0;
	float specPercentage = 0;

	UFUNCTION(BlueprintCallable)
	void StartAttackEvent(int attackCode, float inComboDepth, float addScanTime);
	UFUNCTION(BlueprintCallable)
	void EndAttackEvent(int attackCode);

	void StartUnarmedAttack();
	void EndUnarmedAttack();
	virtual void ProcessEnemyHitEvent(AActor* otherActor);

	UFUNCTION()
	void OnOverlapBeginUnarmed(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere)
	class USphereComponent* rHandOverlap;
	UPROPERTY(EditAnywhere)
	class USphereComponent* lHandOverlap;
	UPROPERTY(EditAnywhere)
	class USphereComponent* rFootOverlap;


#pragma endregion

#pragma region combat overlaps

	UFUNCTION()
	void OnOverlapBeginDamageVolume(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	float AOEDamage = 0;

#pragma endregion

#pragma region melee weapon

	bool hasPersonalWeapon = false;
	class AWeapon* personalWeapon;
	void DestroyPersonalWeapon();

	bool isNobleTracing = false;

	void TraceOn();
	void TraceOn_Init_Failed();

	UFUNCTION(BlueprintCallable)
	void TraceOn_Noble();
	UFUNCTION(BlueprintCallable)
	bool TraceOn_Personal();
	bool TraceOn_Launch();
	UFUNCTION(BlueprintCallable)
	bool TraceOn_Discard();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToSocket(const FString &inName);

#pragma endregion

#pragma region weapon spwaning
	
	FAttachmentTransformRules SNAP_TO_RULE = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);

	TArray<class AWeapon*> spawnedWeapons;

	float weaponSpawnCooldown = .15f;
	const float weaponSpawnCooldownMax = .15f;

	int maxSpawnedWeapons = 15;
	int currentSpawnedWeapons = 0;
	void TrySpawnWeapon();

	FVector GetScanningWeaponSpawnLocation(float numSpawned);

	void InitSpawnedWeapon(bool personal, class AWeapon* wep);

#pragma endregion

#pragma region weapon shooting

	class UClass* breakBowAnimClass;
	class AWeapon* breakBow;
	bool breakBowDestroy = false;
	float breakBowWaitDestroy = .13;
	float maxBreakBowTime = 6.f;

	void ProcessLaunchWeaponMode(float DeltaTime);
	void ProcessBreakBowDestroy(float DeltaTime);
	void DestroyBreakBow();

	void HandleNobleBreakMode();

	bool launchWeaponMode = false;
	UFUNCTION(BlueprintCallable)
	void LaunchNobleWeapon();

	UFUNCTION(BlueprintCallable)
	void InitLaunchWeaponMode();

	float shootWait = .35f;
	void TryShootWeapon();

#pragma endregion

#pragma region trace

	UPROPERTY(EditAnywhere)
	class UNiagaraComponent* availableWeaponsSystem;

	UFUNCTION(BlueprintCallable)
	void EnsureLineTraceCancel();

#pragma region trace volume

	bool isVolumeTracing = false;
	float traceVolumeTime = 2.f;
	const float traceVolumeTimeMax = 2.f;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* traceVolume;
	FVector traceVolumeMaxExtent = FVector(100, 100, 100);

	void InitTraceVolume();
	void CancelTraceVolume();
	void FloatTraceVolume(float DeltaSeconds);

	UFUNCTION()
	void OnOverlapBeginTrace(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEndTrace(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

#pragma endregion

#pragma region trace line

	bool isLineTracing = false;

	UFUNCTION(BlueprintCallable)
	void InitLineTrace();
	void UpdateLineTrace();
	void UpdateTracingWeapons();
	void CancelLineTrace();



	int maxTracingWeapons = 5;
	TArray<AWeapon*> in_tracingWeapons;
	void AddToTracingWeapons(class AWeapon* inWep);
	void RemoveFromTracingWeapons(class AWeapon* inWep);

	UPROPERTY(EditAnywhere)
	class UNiagaraComponent* traceBeam;

	TArray<class UNiagaraComponent*> formingWeapons;

#pragma endregion

#pragma endregion

#pragma region traced weapons

	AWeapon* tempDisplayWeapon;

	TArray<TSubclassOf<class AWeapon>*> done_tracedWeapons;
	int maxTracedWeapons = 15;
	bool AddToTracedWeapons(TSubclassOf<AWeapon>* tracedWep);
	void RemoveFromTracedWeapons(int index);

	int traceWeaponIndex = 0;
	void IncrementTraceWeaponIndexUp();
	void IncrementTraceWeaponIndexDown();
	void IncrementTraceWeaponIndex(float inval);

	void SelectTracedWeapon();
	void SpawnTempDisplayWeapon();
	void DestroyTempDisplayWeapon();
	void TimeoutTempDisplayWeapon(float DeltaTime);

	bool displayingTempWeapon = false;
	float tempDisplayWeaponTimeout = 3.f;
	float tempDisplayWeaponTimeoutMax = 3.f;

	bool traceWeaponMode = false;

#pragma endregion

#pragma region noble

	//UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class AWeapon>*> nobleCache;
	UPROPERTY(EditAnywhere)
	bool freeNobleSpawn = false;

#pragma endregion

#pragma region game mode

	void ToggleGameMode();
	void SetGameMode(EGameMode newGameMode);
	EGameMode currentGameMode = EGameMode::EGM_Standard;

#pragma endregion

#pragma region climbing

	const float maxClimbSurfaceDistance = 100.f;
	const float maxDeltaDist = 50.f;
	UPROPERTY(EditAnywhere)
	float baseClimbSpeed = 125.f;
	UPROPERTY(EditAnywhere)
	float desiredAnchorOffset = 40.f;
	UPROPERTY(EditAnywhere)
	float desiredAnchorDist = 10.f;
	UPROPERTY(EditAnywhere)
	float anchorOverflowLimit = 15.f;
	UPROPERTY(EditAnywhere)
	float desiredAnchorRotAccuracy = 5.f;
	UPROPERTY(EditAnywhere)
	float achorRotAccuracyOverflowLimit = 3.f;

	void TryClamber();
	void TryJumpOffWall();
	UFUNCTION(BlueprintCallable)
	void DoJumpOffWall();

	UFUNCTION(BlueprintCallable)
	float IKClimbTrace(const FName &inSocketName, FVector &offSetVector);

	void AdjustActorLocationForClimb(float DeltaTime);
	void AdjustActorClimbingLocation(float DeltaTime);
	void AdjustActorClimbingRotation(float DeltaTime);

	void MapInputToClimbMovement();
	void ChangeClimbMovementMode(bool climb);
	void AttemptBeginClimb(UPrimitiveComponent* compToClimb, FVector loc, const FHitResult &hit);
	bool AttemptClamber();
	UFUNCTION()
	void OnPawnCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult &Hit);
	void ProcessClimbCooldown(float DeltaTime);
	bool TryFindHit(FHitResult &hit, FVector &horizOffset, FVector searchStart, FVector searchDir, FVector probeDir, int tries = 3);
	bool SetClimbingMatrix(float DeltaTime);
	bool TrySetClimbingAnchor();

	UPROPERTY(EditAnywhere)
	int climbDebugVerbosity = 0;

	bool isRotOverflowAzimuth = false;
	bool isRotOverflow = false;
	bool isAnchorOverflow = false;
	float anchorAdjustDirection = 0;
	bool isClimbing = false;
	bool isClambering = false;
	float climbStartCooldown = .5;
	FHitResult anchorHit, rightHandHit, rightFootHit, leftHandHit, leftFootHit, clamberHit;
	FVector rightHandHorizOffset, leftHandHorizOffset, rightFootHorizOffset, leftFootHorizOffset;
	FVector anchorChord = FVector::ZeroVector;
	FVector2D climbingMove = FVector2D(0,0);

#pragma endregion

public:
	// Sets default values for this character's properties
	AServant();

#pragma region public trace

	bool TryTraceWeapon(class AWeapon* inWep);

#pragma endregion


#pragma region public combat

	UFUNCTION(BlueprintCallable)
	void TryConsumeBufferInput();

	UFUNCTION(BlueprintCallable)
	void ReportAttackCommandSuccess(int code);

	UFUNCTION(BlueprintCallable)
	void ForceAIDoNothing(bool set);

	TArray<USphereComponent*> activeAEOOverlaps;
	TArray<float> activeAOETimes;
	TArray<float> activeAOEDamages;

	USphereComponent* lastActiveAOE;

	void HandleAOEDespawn(float DeltaTime);
	bool activeAOE = false;
	float aoeRemainTime = 0;
	UFUNCTION(BlueprintCallable)
	void DoAOEImmediate(float size, FVector inLoc, bool attachToThis, float aoeExistTime, float damageToDo);

	FORCEINLINE AServant* GetLockedTarget() { return lockedTarget; };
	void ReportLockedTargetDestroyed();

	virtual void DoDamage(float dmgToDo);

	void AddSpecPercent(float specAddAmount);
	void AddCrestPercent(float crestAddAmount);

	void DoHitOther(AActor* otherActor);
	void DoPlayFFB(UForceFeedbackEffect* effectToPlay);
	void DoTakeHit(const FString &specialEffect);

	void AddAttackAffectedActor(AActor* otherActor);
	void EmptyAttackAffectedActors();

	bool IsActorAffectedByCurrentAttack(AActor* otherActor);

	void ReportNobleWeaponUseOver();
	void ReportDestroyedWeapon();

	UFUNCTION(BlueprintCallable)
	void ResetAffectedActors();

	UFUNCTION(BlueprintCallable)
	void StartCombatVolumeTrace();

	UFUNCTION(BlueprintCallable)
	void StartWeaponDangerous();
	UFUNCTION(BlueprintCallable)
	void EndWeaponDangerous();

	void ReportEnemyDestroyed(bool servant = false);


#pragma endregion

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	int cameraInversion = -1;

public:	
	// Called every frame

	class AAIManager* aiManager;

	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void ReportFullTrace(class AWeapon* inWep);
	void ReportTraceFailed(class AWeapon* inWep);

	FORCEINLINE void SetIsInvincible(bool inB) { invincible = inB; };
	FORCEINLINE bool GetIsInvincible() { return invincible; };
	FORCEINLINE bool GetIsPlayer() { return isPlayer; };
	FORCEINLINE float GetHealth() { return health; };

	UFUNCTION(BlueprintCallable)
	float IKFootTrace(const FName &socketName, float traceDist, bool drawDebug);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE class AWeapon* GetPersonalWeapon() { return personalWeapon; };
	
	
};
