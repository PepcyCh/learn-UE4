// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Delegates/IDelegateInstance.h"
#include "ShootingSaveGame.h"
#include "ShootingGameInstance.generated.h"

class IOnlineSubsystem;
class APlayerController;

UCLASS()
class SHOOTING_API UShootingGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FString PlayerName;

	UPROPERTY(VisibleAnywhere)
	uint32 PlayerScore;

	UPROPERTY(VisibleAnywhere)
	TArray<FPlayerScorePair> RankList;
	UPROPERTY(VisibleAnywhere)
	UShootingSaveGame* SaveGameInstance = nullptr;

	UPROPERTY()
	int32 OnlineGamePlayerNumber = 0;
	UPROPERTY()
	int32 OnlineGamePlayerLimit = 0;

public:

	UShootingGameInstance(const FObjectInitializer& ObjectInitializer);

	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetPlayerName(FString Name);
	UFUNCTION(BlueprintCallable, Category = "Player")
	FString GetPlayerName() const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetPlayerScore(int32 Score);
	UFUNCTION(BlueprintCallable, Category = "Player")
	int32 GetPlayerScore() const;

	// UFUNCTION(BlueprintCallable, Category = "RankList")
	// void InsertPlayerScorePair(FString RecordedPlayerName);
	// UFUNCTION(BlueprintCallable, Category = "RankList")
	// FString GetRankListNameStr() const;
	// UFUNCTION(BlueprintCallable, Category = "RankList")
	// FString GetRankListScoreStr() const;
	void InsertGameRecord(const FString& Name, int32 KilledCount, int32 DeathCount);

	void LoadGameRecords();
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void SaveGameRecords();

	bool HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers);
	void FindSessions(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence);
	virtual bool JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult) override;

	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void HostOnlineGame(bool bIsLan, bool bIsPresence, int32 MaxNumPlayers);

	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void StartOnlineGame();
	
	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void FindOnlineGames(bool bIsLan, bool bIsPresence);
	
	// UFUNCTION(BlueprintCallable, Category = "Network|Test")
	// void JoinOnlineGame();
    void JoinOnlineGame(const FOnlineSessionSearchResult& SearchResult);
	
	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void DestroySessionAndLeaveGame();

	int32 GetOnlineGamePlayerNumber() const { return OnlineGamePlayerNumber; }
	int32 GetOnlineGamePlayerLimit() const { return OnlineGamePlayerLimit; }

private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
};
