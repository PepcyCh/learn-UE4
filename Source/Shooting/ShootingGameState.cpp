// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingGameState.h"

AShootingGameState::AShootingGameState()
{
	UE_LOG(LogTemp, Log, TEXT("GameState ctor"));
}


void AShootingGameState::ChangeToState(const EShootingState NewState)
{
	CurrState = NewState;
}

