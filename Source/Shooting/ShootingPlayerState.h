// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShootingPlayerState.generated.h"

UCLASS()
class SHOOTING_API AShootingPlayerState : public APlayerState
{
	GENERATED_BODY()
	
	virtual FString GetPlayerNameCustom() const override;
	
public:
	UPROPERTY(VisibleAnywhere, Replicated)
	FString CustomPlayerName;
	UPROPERTY(VisibleAnywhere, Replicated)
	int32 KillNumber = 0;
	UPROPERTY(VisibleAnywhere, Replicated)
	int32 DeathNumber = 0;

	AShootingPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OverrideWith(APlayerState* PlayerState) override;

	UFUNCTION(Server, Reliable)
    void SetCustomPlayerName_Server(const FString& Name);
	void SetCustomPlayerName_Server_Implementation(const FString& Name);

	int32 GetKillNumber() const { return KillNumber; }
	UFUNCTION(Server, Reliable, WithValidation)
	void IncreaseKillNumber_Server();
	void IncreaseKillNumber_Server_Implementation();
	bool IncreaseKillNumber_Server_Validate();
	UFUNCTION(Server, Reliable)
	void ResetKillNumber_Server();
	void ResetKillNumber_Server_Implementation();

	int32 GetDeathNumber() const { return DeathNumber; }
	UFUNCTION(Server, Reliable, WithValidation)
    void IncreaseDeathNumber_Server();
	void IncreaseDeathNumber_Server_Implementation();
	bool IncreaseDeathNumber_Server_Validate();
	UFUNCTION(Server, Reliable)
    void ResetDeathNumber_Server();
	void ResetDeathNumber_Server_Implementation();

	UFUNCTION(Client, Reliable)
	void InsertRecordToInstance_Client();
};
