// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeWeaponActor.h"

#include "Engine/World.h"
#include "GrenadeActor.h"
#include "GameFramework/Controller.h"
#include "Components/StaticMeshComponent.h"

AGrenadeWeaponActor::AGrenadeWeaponActor()
{
    BulletCount = 1;
}


void AGrenadeWeaponActor::Fire(ACharacter* Character, const FVector& Start, const FVector& Direction)
{
    Super::Fire(Character, Start, Direction);
    Fire_Server(Character, Start, Direction);
}

void AGrenadeWeaponActor::Fire_Server_Implementation(ACharacter* Character, const FVector& Start, const FVector& Direction) const
{
    if (GrenadeClass)
    {
        AGrenadeActor* Grenade = GetWorld()->SpawnActor<AGrenadeActor>(GrenadeClass, Start, FRotator());
        Grenade->SetOwnerController(OwnerController);
        Grenade->SetOwner(OwnerController);
        Grenade->GetGrenadeMeshComponent()->AddImpulse(Direction * 750.0f);
    }
}

