// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/GameState.h"
#include "ShootingGameState.generated.h"

UENUM()
enum class EShootingState
{
	None,
    Waiting,
    InGame,
    End
};

UCLASS()
class SHOOTING_API AShootingGameState : public AGameState
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	EShootingState CurrState = EShootingState::None;

public:
	AShootingGameState();
	
	void ChangeToState(const EShootingState NewState);
	EShootingState GetCurrentState() const { return CurrState; }
};
