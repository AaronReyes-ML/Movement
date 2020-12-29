// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify_PlayParticleEffect.h"
#include "ChldAn_Notify_PlayParticleEffect.generated.h"

/**
 * 
 */
UCLASS()
class MOVEMENT_API UChldAn_Notify_PlayParticleEffect : public UAnimNotify_PlayParticleEffect
{
	GENERATED_BODY()

public:
	void Notify(class USkeletalMeshComponent* MeshComp, class UAnimSequenceBase* Animation) override;
	
};
