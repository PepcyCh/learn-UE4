// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundHelper.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void ASoundHelper::PlaySound(const FString& SoundName) const
{
    if (Sounds.Contains(SoundName))
    {
        const UWorld* World = GetWorld();
        UGameplayStatics::PlaySound2D(Cast<UObject>(World), Sounds[SoundName]);
    }
}

