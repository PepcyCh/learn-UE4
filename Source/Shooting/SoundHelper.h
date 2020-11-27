// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "SoundHelper.generated.h"

UCLASS()
class SHOOTING_API ASoundHelper : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, USoundBase*> Sounds;

	UFUNCTION()
    void PlaySound(const FString& SoundName) const;
};
