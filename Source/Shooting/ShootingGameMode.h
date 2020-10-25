// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShootingGameMode.generated.h"

UCLASS(minimalapi)
class AShootingGameMode : public AGameMode
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	uint32 PlayerCount = 0;

	UPROPERTY(VisibleAnywhere)
	AActor* Target = nullptr;
	UPROPERTY(VisibleAnywhere)
	int32 TargetMoveDirection = 1;

public:
	AShootingGameMode();

	void PostLogin(APlayerController* NewPlayer) override;

	void RestartPlayer(AController* NewPlayer) override;

	void Tick(float DeltaSeconds) override;
};


