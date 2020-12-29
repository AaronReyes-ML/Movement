// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState_TimedParticleEffect.h"
#include "ChildNtfy_TimedParticleEffect.generated.h"

/**
 * 
 */
UCLASS()
class MOVEMENT_API UChildNtfy_TimedParticleEffect : public UAnimNotifyState_TimedParticleEffect
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(class USkeletalMeshComponent * MeshComp, class UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyTick(class USkeletalMeshComponent * MeshComp, class UAnimSequenceBase * Animation, float FrameDeltaTime) override;
	
};
