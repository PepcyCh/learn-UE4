// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "WeaponActor.h"
#include "GameFramework/GameMode.h"
#include "ShootingGameMode.generated.h"

UCLASS(minimalapi)
class AShootingGameMode : public AGameMode
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	uint32 PlayerCount = 0;

	UPROPERTY()
	AActor* Target = nullptr;
	UPROPERTY()
	int32 TargetMoveDirection = 1;

	UPROPERTY()
	float AccumulatedTime = 0.0f;

	UFUNCTION()
	void SpawnRandomWeapon() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<AWeaponActor>> Weapons;

public:
	AShootingGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void Tick(float DeltaSeconds) override;
};


