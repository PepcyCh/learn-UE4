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

	UPROPERTY();
	int32 SpawnedPlayerCount = 0;
	UPROPERTY();
	int32 LoginPlayerCount = 0;
	
	UPROPERTY()
	float AccumulatedTime = 0.0f;
	// UPROPERTY()
	// FTimerHandle StartTimerHandle;
	UPROPERTY()
	FTimerHandle GameTimerHandle;
	UPROPERTY()
	FTimerHandle RestartTimerHandle;

	UPROPERTY()
	FString CurrentMapName = "";

	FString GetCurrentMapName();

	void SpawnRandomWeapon() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GameTime = 90.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<AWeaponActor>> Weapons;

	virtual void BeginPlay() override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Controller) override;

public:
	AShootingGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void Tick(float DeltaSeconds) override;

	void StartGameTimer();
	UFUNCTION()
	void OnGameEnd() const;

	void RestartPlayerDelay(APlayerController* NewPlayer, float DelayTime);
	
};

