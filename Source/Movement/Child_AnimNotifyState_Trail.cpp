// Fill out your copyright notice in the Description page of Project Settings.


#include "Child_AnimNotifyState_Trail.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Servant.h"
#include "Weapon.h"

void UChild_AnimNotifyState_Trail::NotifyBegin(class USkeletalMeshComponent * MeshComp, class UAnimSequenceBase * Animation, float TotalDuration)
{
	bool bError = ValidateInput(MeshComp);

	//IMyTypes::Debug_Print("Notify Begin", 10);

	if (MeshComp->GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	AServant* myOwner = Cast<AServant>(MeshComp->GetOwner());

	if (!myOwner) return;

	//IMyTypes::Debug_Print("Good mesh", 10);

	AWeapon* myOwnerWeapon = myOwner->GetPersonalWeapon();
	if (!myOwnerWeapon)
	{
		//IMyTypes::Debug_Print("No Weapon", 10);
		Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	}
	else
	{
		//IMyTypes::Debug_Print("Good Weapon, expect trail", 10);
		Super::NotifyBegin(myOwnerWeapon->GetWeaponMesh(), Animation, TotalDuration);
	}
}

void UChild_AnimNotifyState_Trail::NotifyEnd(class USkeletalMeshComponent * MeshComp, class UAnimSequenceBase * Animation)
{
	if (MeshComp->GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	//IMyTypes::Debug_Print("Notify End", 10);

	AServant* myOwner = Cast<AServant>(MeshComp->GetOwner());

	if (!myOwner) return;

	//IMyTypes::Debug_Print("Good mesh", 10);

	AWeapon* myOwnerWeapon = myOwner->GetPersonalWeapon();
	if (!myOwnerWeapon)
	{
		//IMyTypes::Debug_Print("No Weapon", 10);
		Super::NotifyEnd(MeshComp, Animation);
	}
	else
	{
		//IMyTypes::Debug_Print("Good Weapon, expect end", 10);
		Super::NotifyEnd(myOwnerWeapon->GetWeaponMesh(), Animation);
	}
}