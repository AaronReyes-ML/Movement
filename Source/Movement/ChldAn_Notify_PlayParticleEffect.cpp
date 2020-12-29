// Fill out your copyright notice in the Description page of Project Settings.


#include "ChldAn_Notify_PlayParticleEffect.h"
#include "Servant.h"
#include "Weapon.h"

void UChldAn_Notify_PlayParticleEffect::Notify(class USkeletalMeshComponent* MeshComp, class UAnimSequenceBase* Animation)
{

	AServant* myOwner = Cast<AServant>(MeshComp->GetOwner());

	if (!myOwner) return;

	AWeapon* myOwnerWeapon = myOwner->GetPersonalWeapon();
	if (!myOwnerWeapon)
	{
		Super::SpawnParticleSystem(MeshComp, Animation);
	}
	else
	{
		Super::SpawnParticleSystem(myOwnerWeapon->GetWeaponMesh(), Animation);
	}
}