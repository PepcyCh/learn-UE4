// Fill out your copyright notice in the Description page of Project Settings.

#include "ShootingPlayerController.h"

#include "ShootingCharacter.h"

void AShootingPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    Possess(GetPawn<AShootingCharacter>());
}