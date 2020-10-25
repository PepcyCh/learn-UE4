// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShootingHUD.generated.h"

UCLASS()
class SHOOTING_API AShootingHUD : public AHUD
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	UFont* Font = nullptr;
	
public:
	AShootingHUD();

	void DrawHUD() override;
};
