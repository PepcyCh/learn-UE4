// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShootingPlayerState.generated.h"

UCLASS()
class SHOOTING_API AShootingPlayerState : public APlayerState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Replicated)
	uint32 ShootingScore = 0;

	UFUNCTION(Server, Reliable, WithValidation)
	void IncreaseScoreServer(uint32 delta = 1);
	void IncreaseScoreServer_Implementation(uint32 delta);
	bool IncreaseScoreServer_Validate(uint32 delta);

	UFUNCTION(Server, Reliable)
	void ResetScoreServer();
	void ResetScoreServer_Implementation();

public:
	UPROPERTY(VisibleAnywhere, Replicated)
	uint32 Id = 0;

	AShootingPlayerState();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void IncreaseScore(uint32 delta = 1);
	void ResetScore();
	uint32 GetShootingScore() const;
};
