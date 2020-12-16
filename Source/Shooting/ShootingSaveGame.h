// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ShootingSaveGame.generated.h"

USTRUCT()
struct FPlayerScorePair
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;
	UPROPERTY()
	int32 KilledCount;
	UPROPERTY()
	int32 DeathCount;
};

UCLASS()
class SHOOTING_API UShootingSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TArray<FPlayerScorePair> RankList;
};
