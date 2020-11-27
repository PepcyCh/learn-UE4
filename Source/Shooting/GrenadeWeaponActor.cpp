// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeWeaponActor.h"

#include "GrenadeActor.h"

void AGrenadeWeaponActor::Fire(ACharacter* Character, const FVector& Start, const FVector& Direction) const
{
    if (GrenadeClass)
    {
        AGrenadeActor* Grenade = GetWorld()->SpawnActor<AGrenadeActor>(GrenadeClass, Start, FRotator());
        Grenade->GetGrenadeMeshComponent()->AddImpulse(Direction * 750.0f);
    }
}
