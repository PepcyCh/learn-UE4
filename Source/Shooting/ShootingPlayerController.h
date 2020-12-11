// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShootingPlayerController.generated.h"

UCLASS()
class SHOOTING_API AShootingPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	FTimerHandle GameTimerHandle;
	UPROPERTY()
	FTimerHandle RetryTimerHandle;

	UPROPERTY()
	class UUserWidget* MainGameUI = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UUserWidget> MainGameUIClass;
	UPROPERTY()
	class UUserWidget* WaitingUI = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UUserWidget> WaitingUIClass;
	UPROPERTY()
	class UUserWidget* EndingUI = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UUserWidget> EndingUIClass;
	UPROPERTY()
	class UUserWidget* DyingUI = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UUserWidget> DyingUIClass;

	void UpdateMainGameUI();
	
public:
	virtual void SetupInputComponent() override;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Client, Reliable)
	void StartOnlineGame_Client();
	UFUNCTION(Client, Reliable)
	void StartGame_Client();
	UFUNCTION(Client, Reliable)
    void EndOnlineGame_Client(const FString& NameStr, const FString& KilledStr, const FString& DeathStr);
	UFUNCTION(Client, Reliable, BlueprintCallable)
    void LeaveOnlineGame_Client();

	int32 GetRemainingTime() const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetPlayerName(const FString& Name);

	UFUNCTION(Client, Reliable)
	void AddWaitingUIToViewport_Client();
	UFUNCTION(Client, Reliable)
    void UpdateWaitingUI_Client(int32 NumCurr, int32 NumMax);
	
	UFUNCTION(Client, Reliable)
    void AddDyingUIToViewport_Client();
};
