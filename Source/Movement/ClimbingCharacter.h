// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ClimbingCharacter.generated.h"

UCLASS()
class MOVEMENT_API AClimbingCharacter : public ACharacter
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere)
		int DEBUG_VAL = 0;

public:
	// Sets default values for this character's properties
	AClimbingCharacter();

	void MoveUp(float axisval);
	void MoveRight(float axisval);
	void AimUp(float axisval);
	void AimRight(float axisval);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	const ECollisionChannel ECC_CLIMB = ECC_GameTraceChannel1;
	const ECollisionChannel ECC_CLIMB_TRACE = ECC_GameTraceChannel2;

	UFUNCTION()
	void OnPawnCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult &Hit);

	UPROPERTY(EditAnywhere)
	class UArrowComponent* forwardPointer;
	UPROPERTY(EditAnywhere)
	class UCameraComponent* camera;
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* cameraBoom;


#pragma region climbing

	bool isClimbing = false;
	bool isClambering = false;
	float climbStartCooldown = .5;
	float climbStartCooldownMax = .5;

	void AttemptBeginClimb(UPrimitiveComponent* compToClimb, FVector loc, FHitResult &hit);
	void AdjustActorToSurface(FHitResult &hit);
	bool SetClimbingMatrix();
	bool AttemptClamber(float DeltaTime);

	void MapInputToClimbMovement();
	void AdjustActorLocationForClimb(float DeltaTime);

	void ChangeClimbMovementMode(bool yes);

	bool TryFindHit(FHitResult &hit, FVector searchStart, FVector searchDir, FVector probeDir, int tries = 3);

	const float maxDeltaDist = 50.f;
	const float maxSearchDist = 100.f;

	bool locInterp = false;
	bool pitchInterp = false;
	float pitchExceed = 0.f;
	const float pitchExceedMax = 5.f;
	bool yawInterp = false;
	float yawExceed = 0.f;
	const float yawExceedMax = 5.f;
	
	FVector climbVector;
	FHitResult anchorHit, rightHandHit, rightFootHit, leftHandHit, leftFootHit, clamberHit;
	FVector anchorChord = FVector::ZeroVector;

	float GetAngleToComponent(UPrimitiveComponent* otherComp);
	float GetAngleToLocation(FVector loc);
	float GetAngleToVector(FVector dir);
	float GetAngleVectorVector(FVector first, FVector last);

#pragma endregion


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void EndClamber();
	
};
