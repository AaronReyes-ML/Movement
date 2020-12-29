// Fill out your copyright notice in the Description page of Project Settings.


#include "NobleAurora.h"
#include "Servant.h"
#include "Kismet/GameplayStatics.h"


void ANobleAurora::DoAuroraAOE(UParticleSystem* aoeSys)
{
	if (!aoeSys) return;
	if (!weaponOwner) return;

	FVector ownerLoc = weaponOwner->GetActorLocation();
	FVector fwdDir = weaponOwner->GetActorForwardVector();

	int angle = 30;


	for (int i = 0; i < 5; i++)
	{
		FVector spawnLoc = fwdDir.RotateAngleAxis(angle, FVector(0, 0, 1)) * 125;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), aoeSys, (ownerLoc + spawnLoc) * FVector(1, 1, 0), FRotator(0, weaponOwner->GetActorRotation().Yaw,0));
		weaponOwner->DoAOEImmediate(50, (ownerLoc + spawnLoc) * FVector(1,1,0), false, .2, 1.2);
		angle -= 15;
	}
}
