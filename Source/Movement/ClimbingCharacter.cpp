// Fill out your copyright notice in the Description page of Project Settings.

#include "ClimbingCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Components/ArrowComponent.h"
#include "ClimbAnimInstance.h"
#include "Engine.h"

#pragma region debug print

void DebugPrint(const FString &inStr, int printTime = 1)
{
	GEngine->AddOnScreenDebugMessage(-1, printTime, FColor::Emerald, inStr);
}

void DebugPrint(const float inVal, int printTime = 1)
{
	GEngine->AddOnScreenDebugMessage(-1, printTime, FColor::Emerald, FString::SanitizeFloat(inVal));
}

void DebugPrint(const int inVal, int printTime = 1)
{
	GEngine->AddOnScreenDebugMessage(-1, printTime, FColor::Emerald, FString::FromInt(inVal));
}

#pragma endregion

#pragma region construct

// Sets default values
AClimbingCharacter::AClimbingCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	forwardPointer = CreateDefaultSubobject<UArrowComponent>("Forward dir");
	forwardPointer->SetupAttachment(RootComponent);
	forwardPointer->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
	forwardPointer->bHiddenInGame = false;

	cameraBoom = CreateDefaultSubobject<USpringArmComponent>("Camera Boom");
	cameraBoom->SetupAttachment(RootComponent);
	cameraBoom->SetRelativeRotation(FRotator(0, 0, 0));
	cameraBoom->SetRelativeLocation(FVector::ZeroVector);
	cameraBoom->TargetArmLength = 400;

	camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	camera->SetupAttachment(cameraBoom, cameraBoom->SocketName);
	camera->SetRelativeLocation(FVector::ZeroVector);
	camera->SetRelativeRotation(FRotator::ZeroRotator);
	camera->bUsePawnControlRotation = false;
	

}

void AClimbingCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AClimbingCharacter::OnPawnCollision);

}

void AClimbingCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAxis("MoveRight", this, &AClimbingCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AClimbingCharacter::MoveUp);
	PlayerInputComponent->BindAxis("LookUp", this, &AClimbingCharacter::AimUp);
	PlayerInputComponent->BindAxis("Turn", this, &AClimbingCharacter::AimRight);

}

#pragma endregion

void AClimbingCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (climbStartCooldown > 0)
	{
		climbStartCooldown -= DeltaTime;
	}

	//DebugPrint("C++  --- Is Climbing: " + FString::FromInt(isClimbing) + ", Is Clambering: " + FString::FromInt(isClambering));

	if (isClimbing)
	{
		if (!isClambering)
		{
			AdjustActorLocationForClimb(DeltaTime);
			MapInputToClimbMovement();
		}
	}
}

#pragma region movement and aim

void AClimbingCharacter::MoveUp(float axisval)
{
	if ((Controller) && (axisval != 0.0f))
	{
		if (!isClimbing)
		{
			AddMovementInput(GetActorForwardVector(), 5 * axisval);
		}
	}
}

void AClimbingCharacter::MoveRight(float axisval)
{
	if (Controller && axisval != 0.0f)
	{
		if (!isClimbing)
		{
			AddControllerYawInput(2.75 * axisval);
		}
	}
}

void AClimbingCharacter::AimUp(float axisval)
{
	if (Controller && axisval != 0.0f)
	{
		cameraBoom->AddRelativeRotation(FRotator(2 * -axisval, 0, 0));
	}
}

void AClimbingCharacter::AimRight(float axisval)
{
	if (Controller && axisval != 0.0f)
	{
		cameraBoom->AddRelativeRotation(FRotator(0, 2 * axisval, 0));
	}
}

#pragma endregion

#pragma region collision

void AClimbingCharacter::OnPawnCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult &Hit)
{
	if (OtherComp->GetCollisionObjectType() == ECC_CLIMB && !isClimbing)
	{
		FHitResult myHit = Hit;
		AttemptBeginClimb(OtherComp, Hit.ImpactPoint, myHit);
	}
	else if ((OtherComp->GetCollisionObjectType() == ECC_WorldStatic) && isClimbing && climbStartCooldown <= 0)
	{
		FHitResult myHit = FHitResult();
		GetWorld()->LineTraceSingleByObjectType(myHit, GetCapsuleComponent()->GetComponentLocation(), GetCapsuleComponent()->GetComponentLocation() - GetCapsuleComponent()->GetUpVector() * 150, ECC_WorldStatic);
		if (myHit.IsValidBlockingHit())
		{
			ChangeClimbMovementMode(false);
		}
	}
}

#pragma endregion

#pragma region climbing

void AClimbingCharacter::MapInputToClimbMovement()
{
	float upValue = GetInputAxisValue("MoveForward");
	float rightValue = GetInputAxisValue("MoveRight");

	if (!FMath::IsNearlyZero(upValue, 0.01f))
	{
		climbVector = FVector(0, 0, upValue > 0 ? 1 : -1);
	}
	else if (!FMath::IsNearlyZero(rightValue, 0.01f))
	{
		climbVector = FVector(rightValue > 0 ? 1 : -1, 0, 0);
	}
	else
	{
		climbVector = FVector::ZeroVector;
	}

	UPrimitiveComponent* otherComp = anchorHit.GetComponent();

	/*
	if (otherComp)
	{
		if (otherComp->IsSimulatingPhysics())
		{
			if (climbVector != FVector::ZeroVector)
			{
				DebugPrint("UnWelding-------------------------------", 2);
				GetCapsuleComponent()->UnWeldFromParent();
			}
			else if (!GetCapsuleComponent()->IsWelded())
			{
				DebugPrint("-------------------------------Welding", 2);
				GetCapsuleComponent()->WeldTo(otherComp);
			}
		}
	}
	*/

	UClimbAnimInstance* myInst = Cast<UClimbAnimInstance>(GetMesh()->GetAnimInstance());
	if (myInst)
	{
		myInst->Climb(climbVector);
		myInst->ReportPositions(rightHandHit, leftHandHit, rightFootHit, leftFootHit);
	}

}

void AClimbingCharacter::AdjustActorLocationForClimb(float DeltaTime)
{
	if (!SetClimbingMatrix())
	{
		if (DEBUG_VAL > 0) DebugPrint("Climbing fail", 10);
		ChangeClimbMovementMode(false);
		return;
	}

	/*
	if (GetCapsuleComponent()->IsWelded()) return;
	DebugPrint("Not welded");
	*/

	anchorChord = anchorHit.ImpactPoint - GetCapsuleComponent()->GetComponentLocation();
	if (anchorChord.Size() > 60 || locInterp)
	{
		if (DEBUG_VAL > 3) DebugPrint("Loc delta: " + FString::SanitizeFloat(anchorChord.Size()));
		if (!locInterp) locInterp = true;
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), anchorHit.ImpactPoint, DeltaTime, 1));
		if (FVector::PointsAreNear(GetActorLocation(), anchorHit.ImpactPoint,40))
		{
			if (DEBUG_VAL > 3)DebugPrint("Setting loc interp false");
			locInterp = false;
		}
	}
	float angle = GetAngleToVector(anchorHit.ImpactNormal);
	float delta = FMath::Abs(angle - 180);
	if (delta > 10 || pitchInterp)
	{
		if (DEBUG_VAL > 3) DebugPrint("Pitch delta: " + FString::SanitizeFloat(delta));
		if (!pitchInterp) pitchInterp = true;
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), (-anchorHit.ImpactNormal).ToOrientationRotator(), DeltaTime, 1));
		if (GetActorRotation().Equals((-anchorHit.ImpactNormal).ToOrientationRotator(),10))
		{
			if (DEBUG_VAL > 3) DebugPrint("Setting pitch interp false");
			pitchInterp = false;
		}
	}
	angle = GetAngleVectorVector(GetCapsuleComponent()->GetRightVector(), anchorHit.ImpactNormal);
	delta = FMath::Abs(angle - 90);
	if (delta > 10 || yawInterp)
	{
		if (DEBUG_VAL > 3) DebugPrint("Yaw delta: " + FString::SanitizeFloat(delta));
		if (!yawInterp) yawInterp = true;
		Controller->SetControlRotation(FMath::RInterpTo(GetActorRotation(), (-anchorHit.ImpactNormal).ToOrientationRotator(), DeltaTime, 1));
		if (Controller->GetControlRotation().Equals((-anchorHit.ImpactNormal).ToOrientationRotator(), 10))
		{
			if (DEBUG_VAL > 3) DebugPrint("Setting yaw interp false");
			yawInterp = false;
		}
	}
}

void AClimbingCharacter::ChangeClimbMovementMode(bool clmb)
{
	if (clmb)
	{
		UClimbAnimInstance* myInst = Cast<UClimbAnimInstance>(GetMesh()->GetAnimInstance());
		if (myInst) myInst->SetClimbing(true);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
		climbStartCooldown = climbStartCooldownMax;
		isClimbing = true;
	}
	else
	{
		UClimbAnimInstance* myInst = Cast<UClimbAnimInstance>(GetMesh()->GetAnimInstance());
		if (myInst) myInst->SetClimbing(false);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		FRotator rot = GetActorRotation();
		rot.Pitch = rot.Roll = 0;
		SetActorRotation(rot);
		isClimbing = false;
		//if (GetCapsuleComponent()->IsWelded()) GetCapsuleComponent()->UnWeldFromParent();
	}
}

void AClimbingCharacter::AttemptBeginClimb(UPrimitiveComponent* compToClimb, FVector loc, FHitResult &hit)
{
	if (isClambering) return;

	float angle = GetAngleToLocation(loc);
	if (angle < 60.f && angle > -60.f)
	{
		if (SetClimbingMatrix())
		{
			ChangeClimbMovementMode(true);
		}
	}
}

void AClimbingCharacter::AdjustActorToSurface(FHitResult &hit)
{
	FVector dir = hit.ImpactNormal;
	dir = -dir;
}

bool AClimbingCharacter::SetClimbingMatrix()
{
	FVector acLoc = GetCapsuleComponent()->GetComponentLocation();
	FVector acFwd = GetCapsuleComponent()->GetForwardVector();
	FVector acR = GetCapsuleComponent()->GetRightVector();
	FVector acU = GetCapsuleComponent()->GetUpVector();

	if (DEBUG_VAL > 1)
	{
		DrawDebugLine(GetWorld(), acLoc, acLoc + acFwd * maxSearchDist, FColor::Red, false, 10);
	}

	GetWorld()->LineTraceSingleByChannel(anchorHit, acLoc, acLoc + acFwd * maxSearchDist, ECC_CLIMB_TRACE);
	if (!anchorHit.IsValidBlockingHit())
	{
		if (DEBUG_VAL > 0) DebugPrint("Anchor Fail", 10);
		return false;
	}

	bool a = false;
	bool b = false;
	if (!TryFindHit(rightHandHit, acLoc + (acU * maxDeltaDist), acR, -anchorHit.ImpactNormal))
	{
		a = true;
	}
	if (!TryFindHit(leftHandHit, acLoc + (acU * maxDeltaDist), -acR, -anchorHit.ImpactNormal)) 
	{
		b = true;
	}

	if (a && b)
	{
		if (DEBUG_VAL > 0) DebugPrint("Hand Fail", 10);
		if (!isClimbing) return false;
		if (!AttemptClamber(0)) return false;
		return true;
	}

 	TryFindHit(rightFootHit, acLoc - (acU * maxDeltaDist * .5), acR, -anchorHit.ImpactNormal);
	TryFindHit(leftFootHit, acLoc - (acU * maxDeltaDist * .5), -acR, -anchorHit.ImpactNormal);

	if (DEBUG_VAL > 0)
	{
		DrawDebugSphere(GetWorld(), anchorHit.ImpactPoint, 5, 4, FColor::Red, false, 1);
		DrawDebugSphere(GetWorld(), rightHandHit.ImpactPoint, 5, 4, FColor::Blue, false, 1);
		DrawDebugSphere(GetWorld(), leftHandHit.ImpactPoint, 5, 4, FColor::Blue, false, 1);
		DrawDebugSphere(GetWorld(), rightFootHit.ImpactPoint, 5, 4, FColor::Blue, false, 1);
		DrawDebugSphere(GetWorld(), leftFootHit.ImpactPoint, 5, 4, FColor::Blue, false, 1);
	}

	return true;
}

bool AClimbingCharacter::AttemptClamber(float DeltaTime)
{
	if (!TryFindHit(clamberHit, GetCapsuleComponent()->GetComponentLocation() + GetCapsuleComponent()->GetUpVector() * maxDeltaDist, GetCapsuleComponent()->GetForwardVector(), -GetCapsuleComponent()->GetUpVector()))
	{
		if (DEBUG_VAL > 0) DebugPrint("Clamber location fail", 10);
		return false;
	}

	isClambering = true;
	UClimbAnimInstance* myInst = Cast<UClimbAnimInstance>(GetMesh()->GetAnimInstance());
	if (myInst)
	{
		myInst->DoClamber();
	}
	return true;
}

void AClimbingCharacter::EndClamber()
{
	isClambering = false;
	ChangeClimbMovementMode(false);
}

bool AClimbingCharacter::TryFindHit(FHitResult &hit, FVector searchStart, FVector searchDirection, FVector probeDir, int tries)
{
	if (tries <= 0) return false;
	if (tries > 5) tries = 5;

	searchDirection.Normalize();

	hit = FHitResult();
	bool found = false;

	for (int i = 1; i < tries + 1; i++)
	{
		FVector placeToSearch = searchStart + searchDirection * (maxDeltaDist / i);
		FVector dirToSearch = placeToSearch + probeDir * maxSearchDist;

		if (DEBUG_VAL > 1)
		{
			//DrawDebugLine(GetWorld(), placeToSearch, dirToSearch, FColor::Red, false, 1);
		}


		GetWorld()->LineTraceSingleByChannel(hit, placeToSearch, dirToSearch, ECC_CLIMB_TRACE);
		if (hit.IsValidBlockingHit())
		{
			found = true;
			break;
		}
	}

	if (!found) return false;
	else return true;
}

#pragma endregion

#pragma region utility

float AClimbingCharacter::GetAngleToComponent(UPrimitiveComponent* otherComp)
{
	if (!otherComp) return -1;

	FVector actorLoc = GetCapsuleComponent()->GetComponentLocation();
	FVector otherLoc = otherComp->GetComponentLocation();

	actorLoc.Z = 0;
	otherLoc.Z = 0;

	FVector actorForward = GetCapsuleComponent()->GetForwardVector();
	FVector toTarget = otherLoc - actorLoc;

	actorForward.Normalize();
	toTarget.Normalize();

	float angle = FMath::Acos(FVector::DotProduct(actorForward, toTarget));
	angle = FMath::RadiansToDegrees(angle);

	return angle;
}

float AClimbingCharacter::GetAngleToLocation(FVector loc)
{
	if (loc == FVector::ZeroVector) return -1;

	FVector actorLoc = GetCapsuleComponent()->GetComponentLocation();
	FVector otherLoc = loc;

	actorLoc.Z = 0;
	otherLoc.Z = 0;

	FVector actorForward = GetCapsuleComponent()->GetForwardVector();
	FVector toTarget = otherLoc - actorLoc;

	actorForward.Normalize();
	toTarget.Normalize();

	//DrawDebugLine(GetWorld(), GetCapsuleComponent()->GetComponentLocation(), GetCapsuleComponent()->GetComponentLocation() + actorForward * 300, FColor::Blue, true, 50);
	//DrawDebugLine(GetWorld(), GetCapsuleComponent()->GetComponentLocation(), GetCapsuleComponent()->GetComponentLocation() + toTarget * 300, FColor::Red, true, 50);

	float angle = FMath::Acos(FVector::DotProduct(actorForward, toTarget));
	angle = FMath::RadiansToDegrees(angle);

	//DebugPrint("Angle: " + FString::SanitizeFloat(angle), 10);

	return angle;
}

float AClimbingCharacter::GetAngleToVector(FVector loc)
{
	if (loc == FVector::ZeroVector) return -1;

	FVector actorLoc = GetCapsuleComponent()->GetComponentLocation();

	FVector actorForward = GetCapsuleComponent()->GetForwardVector();
	FVector targetDir = loc;

	actorForward.Normalize();
	targetDir.Normalize();

	//DrawDebugLine(GetWorld(), actorLoc, actorLoc + actorForward * 200, FColor::Blue, false, 1);
	//DrawDebugLine(GetWorld(), actorLoc, actorLoc + targetDir * 200, FColor::Emerald, false, 1);

	float angle = FMath::Acos(FVector::DotProduct(actorForward, targetDir));
	angle = FMath::RadiansToDegrees(angle);

	return angle;

}

float AClimbingCharacter::GetAngleVectorVector(FVector first, FVector last)
{
	if (first == FVector::ZeroVector || last == FVector::ZeroVector) return -1;

	first.Normalize();
	last.Normalize();

	float angle = FMath::Acos(FVector::DotProduct(first, last));
	return FMath::RadiansToDegrees(angle);
}

#pragma endregion