// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"


USTRUCT()
struct FAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool processThis = false;
	UPROPERTY(EditAnywhere)
	bool canControlMove = false;
	UPROPERTY(EditAnywhere)
	bool canIgnorePawn = false;
	UPROPERTY(EditAnywhere)
	FString hitEffect = "None";
	UPROPERTY(EditAnywhere)
	bool forceMaxMovement = false;
};

UCLASS()
class MOVEMENT_API AWeapon : public AActor
{
	GENERATED_BODY()

private:

	typedef enum EWeaponClass
	{
		EWC_Standard = 0,
		EWC_Noble = 9999
	};

	typedef enum EWeaponMode
	{
		EWM_None = 0,
		EWM_Preview = 1,
		EWM_Held = 2,
		EWM_Projectile = 3,
		EWM_Disabled = 4
	};

	void JitterSelfLoc(float DeltaTime);

public:	
	// Sets default values for this actor's properties
	AWeapon();
	AWeapon(FVector initSpawnVector, FRotator initSpawnDir);

	bool dumb = false;
	void DoWeaponDestroyAndRemove();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UMaterialInstanceDynamic* weaponMat;

	bool weaponDestroyable = false;
	float weaponDurability = 1.f;

	bool self = false;

	UFUNCTION()
	void OnWeaponCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult &Hit);

	UPROPERTY(EditAnywhere)
	class USkeletalMesh* weaponSourceMesh;
	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* weaponMesh;

	EWeaponMode currentWeaponMode = EWeaponMode::EWM_None;

	FVector weaponSize = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* searchOverlap;

	float selfDestroyCountdown = 15.f;
	bool isCountingDown = false;
	void HandleSelfDestroy(float DeltaTime);

	class AServant* weaponOwner;
	class AServant* scanningActor;

	UFUNCTION()
	void OnOverlapBeginOther(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEndOther(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ApplyWeaponDurabilityDamage();
	void WeaponDestroyByDurability();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FRotator actorToWeaponDeltaRotator = FRotator::ZeroRotator;

#pragma region weapon properties

	UPROPERTY(EditAnywhere)
	class UForceFeedbackEffect* weaponFeedback;
	UPROPERTY(EditAnywhere)
	float wipeStart = 0;
	UPROPERTY(EditAnywhere)
	float wipeEnd = 0;

	UPROPERTY(EditAnywhere)
	bool nobleWeapon = false;

	UPROPERTY(EditAnywhere)
	bool takeDurabilityDamage = true;
	UPROPERTY(EditAnywhere)
	bool forceCanScan = false;

	UPROPERTY(EditAnywhere)
	float weaponDRH = .05f;
	UPROPERTY(EditAnywhere)
	float weaponDRH_MaxDeviance = 3.f;

	UPROPERTY(EditAnywhere)
	float weaponDamage = .3f;
	UPROPERTY(EditAnywhere)
	float weaponDamage_MaxDeviance = .5f;

#pragma endregion

#pragma region attacks

	EMovementMode cachedMovementMode = EMovementMode::MOVE_Walking;

	UPROPERTY(EditAnywhere)
	class UCurveVector* attack1MovementCurve;
	UPROPERTY(EditAnywhere)
	class UCurveVector* attack2MovementCurve;
	UPROPERTY(EditAnywhere)
	class UCurveVector* attack3MovementCurve;
	UPROPERTY(EditAnywhere)
	class UCurveVector* attack4MovementCurve;
	UPROPERTY(EditAnywhere)
	class UCurveVector* attack5MovementCurve;
	UPROPERTY(EditAnywhere)
	class UCurveVector* attack6MovementCurve;

	UPROPERTY(EditAnywhere)
	class UCurveFloat* attackDamageCurve;

	bool processThisAttack = false;
	FAttackData currentAttackData = FAttackData();
	UPROPERTY(EditAnywhere)
	TArray<FAttackData> attackDataArray;

	class USphereComponent* attackOverlapVolume;


#pragma endregion

#pragma region combat

	bool weaponDanger = false;
	float weaponDamageVelocity = 0;
	FVector prevFramePointLoc = FVector::ZeroVector;

	void DoAOE();

	class UCurveVector* testCurve;

	void InitHeldMode(class AServant* owner);

	bool isProcessingAttack = false;
	float timeInAttack = 0;
	int currentAttackCode = 0;
	float currentComboDepth = 0;
	float currentAddScanTime = 0;
	FString currentAttackSpecialEffect = "None";

	virtual void StartAttackHandler(int inAttackCode, float inComboDepth, float addScanTime);
	void EndAttackHandler(int inAttackCode);

	void StartWeaponDangerous();
	void EndWeaponDangerous();

	void ProcessAttack(float DeltaTime);

	void ProcessAttack1(float DeltaTime);
	void ProcessAttack2(float DeltaTime);
	virtual void ProcessAttack3(float DeltaTime);
	void ProcessAttack4(float DeltaTime);
	void ProcessAttack5(float DeltaTime);
	void ProcessAttack6(float DeltaTime);

	void ProcessEnemyHitEvent(AActor* otherActor);

	void DoWeaponDropAndRemove();


#pragma endregion

#pragma region projectile

	bool noUpdateCollision = false;
	FQuat initQuat = FQuat();
	FQuat finalQuat = FQuat();

	AActor* projectileTarget;
	FVector directionToTarget = FVector::ZeroVector;

	void SetProjectileCollision();

	FVector GetPositionBetweenWeapon();
	FVector GetWeaponDir();

	bool initProjectileDone = false;
	float initProjectileTime = .2;
	void IdentifyProjectileTarget();
	FVector GetDirectionToTarget();
	void DoInitProjectileSequence(float DeltaTime);
	void InitProjectileMode(FVector inTangent, class AServant* other, class AActor* trgt, float addInitTime = 0);

	void ProjectileSeekTarget();

	float projectileSeekWaitTime = 0;
	void InitSelfDestroySequence();
	void InitProjectileImmediate(class AServant* owner, class AActor* trgt);

#pragma endregion

#pragma region trace

	EWeaponClass weaponClass = EWeaponClass::EWC_Standard;

	UPROPERTY(EditAnywhere)
	float selfMult = 1;
	UPROPERTY(EditAnywhere)
	float selfMult2 = 0;
	UPROPERTY(EditAnywhere)
	float selfMult3 = 1;

	float accumulateScanTime = 0;
	bool inSpawnSequence = false;
	float spawnSequenceTime = 0.8f;
	UPROPERTY(EditAnywhere)
	float spawnSequenceTimeMax = .8f;
	void KickOffSpawnSequence();
	void TimeoutSpawnSequence(float DeltaTime);


	class UNiagaraComponent* formSelfComponent;
	class UNiagaraComponent* traceSelfComponent;
	class UNiagaraComponent* traceFinishComponent;


	UPROPERTY(EditAnywhere)
	class UBlendSpace* movementAnim;
	UPROPERTY(EditAnywhere)
	class UAnimMontage* traceOnMontage;
	UPROPERTY(EditAnywhere)
	class UAnimSequence* idleAnim;
	UPROPERTY(EditAnywhere)
	class UAnimMontage* weaponAttackMontage;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* traceSelfSystem;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* formSystem;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* failSystem;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* successSystem;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* explodeSystem;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* shedSystem;
	UPROPERTY(EditAnywhere)
	class UParticleSystem* explodeSystemNrmParticle;
	UPROPERTY(EditAnywhere)
	class UParticleSystem* weaponImpactParticle;


	class UAnimMontage* damageMontageTest;

	FORCEINLINE class UNiagaraSystem* GetFormSystem() { return formSystem; };
	FORCEINLINE class UNiagaraSystem* GetExplodeSystem() { return failSystem; };

	FORCEINLINE void SetScanner(class AServant* inScanningActor) { scanningActor = inScanningActor; };

	UPROPERTY(EditAnywhere)
	float validMissTime = .2f;
	UPROPERTY(EditAnywhere)
	float validMissTimeMax = .2f;
	UPROPERTY(EditAnywhere)
	float timeToFullScan = 1.5f;
	UPROPERTY(BlueprintReadOnly)
	float scanPercent = 0.f;
	UPROPERTY(EditAnywhere)
	bool infiniteScan = false;
	bool canScan = false;
	bool isScanning = false;
	UPROPERTY(BlueprintReadOnly)
	bool isScannedByBox = false;
	UPROPERTY(BlueprintReadOnly)
	bool lastScanMiss = true;

	float totalTime = 0.0f;
	bool jitterSelf = false;
	FORCEINLINE void SetJitterSelf(bool in) { jitterSelf = in; };

	bool hideWeaponMesh = false;

	void UseWeaponAsActorOrientation(const FVector &setDir);
	void OrientSelfToWeapon(float mult = 1.f);

	void InitTraceParticles();
	void InitCompleteParticles(bool success);

	void AddScanTime(float DeltaSeconds);
	void AddMissTime(float DeltaSeconds);
	void ResetScan();
	void SetScanLocation();

	void SelfTraceBegin();
	void SelfTraceComplete();
	void SelfTraceFail();

	void SetMeshHidden(bool hide);

	void StartPreviewParticle();
	void DestroyPreviewParticle();

	UPROPERTY(EditAnywhere)
	AActor* debugTarget;
	UPROPERTY(EditAnywhere)
	bool debugFaceTarget = false;

#pragma endregion

	FORCEINLINE float GetWeaponDurability() { return weaponDurability; };

	UFUNCTION(BlueprintCallable)
	FORCEINLINE class USkeletalMeshComponent* GetWeaponMesh() { return weaponMesh; };
	FORCEINLINE class USkeletalMesh* GetSourceMesh() { return weaponSourceMesh; };
};
