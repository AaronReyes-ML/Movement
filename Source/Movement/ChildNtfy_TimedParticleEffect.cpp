// Fill out your copyright notice in the Description page of Project Settings.


#include "ChildNtfy_TimedParticleEffect.h"
#include "Servant.h"
#include "Weapon.h"

void UChildNtfy_TimedParticleEffect::NotifyBegin(class USkeletalMeshComponent * MeshComp, class UAnimSequenceBase * Animation, float TotalDuration)
{

	//IMyTypes::Debug_Print("Notify Begin", 10);

	if (MeshComp->GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	AServant* myOwner = Cast<AServant>(MeshComp->GetOwner());

	if (!myOwner) return;


	AWeapon* myOwnerWeapon = myOwner->GetPersonalWeapon();
	if (!myOwnerWeapon)
	{
		Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	}
	else
	{
		Super::NotifyBegin(myOwnerWeapon->GetWeaponMesh(), Animation, TotalDuration);
	}
}

void UChildNtfy_TimedParticleEffect::NotifyTick(class USkeletalMeshComponent * MeshComp, class UAnimSequenceBase * Animation, float FrameDeltaTime)
{
	if (MeshComp->GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	AServant* myOwner = Cast<AServant>(MeshComp->GetOwner());

	if (!myOwner) return;

	AWeapon* myOwnerWeapon = myOwner->GetPersonalWeapon();
	if (!myOwnerWeapon)
	{
		Super::NotifyEnd(MeshComp, Animation);
	}
	else
	{
		Super::NotifyEnd(myOwnerWeapon->GetWeaponMesh(), Animation);
	}
}