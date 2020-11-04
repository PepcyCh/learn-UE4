// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ShootingSaveGame.generated.h"

USTRUCT()
struct FPlayerScorePair
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FString Name;
	UPROPERTY(VisibleAnywhere)
	uint32 Score;
};

UCLASS()
class SHOOTING_API UShootingSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere)
	TArray<FPlayerScorePair> RankList;
};
