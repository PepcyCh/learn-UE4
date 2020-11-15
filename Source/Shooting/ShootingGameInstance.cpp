// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "../Plugins/Online/OnlineSubsystem/Source/Public/Online.h"
#include "../Plugins/Online/OnlineSubsystemUtils/Source/OnlineSubsystemUtils/Public/OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"

UShootingGameInstance::UShootingGameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UShootingGameInstance::OnCreateSessionComplete);
    OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UShootingGameInstance::OnStartSessionComplete);
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UShootingGameInstance::OnFindSessionsComplete);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UShootingGameInstance::OnJoinSessionComplete);
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UShootingGameInstance::OnDestroySessionComplete);
}

void UShootingGameInstance::Init()
{
	SaveGameInstance = Cast<UShootingSaveGame>(UGameplayStatics::CreateSaveGameObject(UShootingSaveGame::StaticClass()));
	LoadGameRecords();
}

void UShootingGameInstance::SetPlayerName(FString Name)
{
	PlayerName = Name;
}
FString UShootingGameInstance::GetPlayerName() const
{
	return PlayerName;
}

void UShootingGameInstance::SetPlayerScore(int32 Score)
{
	PlayerScore = Score;
}
int32 UShootingGameInstance::GetPlayerScore() const
{
	return PlayerScore;
}

void UShootingGameInstance::InsertPlayerScorePair(FString RecordedPlayerName)
{
	const static int32 MAX_RECORD = 10;
	if (RankList.Num() >= MAX_RECORD && PlayerScore < RankList[MAX_RECORD - 1].Score)
	{
		return;
	}

	FPlayerScorePair NewRecord { RecordedPlayerName, PlayerScore };
	int32 Rank = RankList.Num();
	for (int32 i = 0; i < RankList.Num(); i++)
	{
		if (NewRecord.Score > RankList[i].Score)
		{
			Rank = i;
			break;
		}
	}
	RankList.Add(FPlayerScorePair{ RecordedPlayerName, PlayerScore });
	if (Rank == RankList.Num())
	{
		return;
	}
	for (int32 i = RankList.Num() - 1; i > Rank; i--)
	{
		RankList[i] = RankList[i - 1];
	}
	RankList[Rank] = NewRecord;
	if (RankList.Num() > MAX_RECORD)
	{
		RankList.Pop();
	}
}
FString UShootingGameInstance::GetRankListNameStr() const
{
	FString Res;
	for (FPlayerScorePair Pair : RankList)
	{
		Res += Pair.Name + "\n";
	}
	return Res;
}
FString UShootingGameInstance::GetRankListScoreStr() const
{
	FString Res;
	for (FPlayerScorePair Pair : RankList)
	{
		Res += FString::FromInt(Pair.Score) + "\n";
	}
	return Res;
}

void UShootingGameInstance::LoadGameRecords()
{
	UShootingSaveGame* SaveInstance = Cast<UShootingSaveGame>(UGameplayStatics::LoadGameFromSlot("ScoreRecords", 0));
	if (SaveInstance)
	{
		RankList = SaveInstance->RankList;
	}
}
void UShootingGameInstance::SaveGameRecords()
{
	if (!SaveGameInstance)
	{
		return;
	}
	SaveGameInstance->RankList = RankList;
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, "ScoreRecords", 0);
}

bool UShootingGameInstance::HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		return false;
	}

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	if (!Sessions.IsValid() || !UserId.IsValid())
	{
		return false;
	}

	SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = true;
	SessionSettings->bUsesPresence = bIsPresence;
	SessionSettings->NumPublicConnections = MaxNumPlayers;
	SessionSettings->NumPrivateConnections = 0;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings->Set(SETTING_MAPNAME, FString("ThirdPersonExampleMap"), EOnlineDataAdvertisementType::ViaOnlineService);

	OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
	return Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
}

void UShootingGameInstance::FindSessions(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (!OnlineSub)
	{
		OnFindSessionsComplete(false);
	}

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	if (!Sessions.IsValid() || !UserId.IsValid())
	{
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->PingBucketSize = 50;
	if (bIsPresence)
	{
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
	}


	OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

	TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();
	Sessions->FindSessions(*UserId, SearchSettingsRef);
}

bool UShootingGameInstance::JoinCertainSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		return false;
	}

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	if (!Sessions.IsValid() || !UserId.IsValid())
	{
		return false;
	}

	OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
	return Sessions->JoinSession(*UserId, SessionName, SearchResult);
}

void UShootingGameInstance::StartOnlineGame(bool bIsLan, bool bIsPresence, int32 MaxNumPlayers)
{
	DestroySessionAndLeaveGame();
	ULocalPlayer* Player = GetFirstGamePlayer();
	HostSession(Player->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, bIsLan, bIsPresence, MaxNumPlayers);
}

void UShootingGameInstance::FindOnlineGames(bool bIsLan, bool bIsPresence)
{
	ULocalPlayer* Player = GetFirstGamePlayer();
	FindSessions(Player->GetPreferredUniqueNetId().GetUniqueNetId(), bIsLan, bIsPresence);
}

void UShootingGameInstance::JoinOnlineGame()
{
	ULocalPlayer* Player = GetFirstGamePlayer();

	FOnlineSessionSearchResult SearchResult;
	if (SessionSearch->SearchResults.Num() > 0)
	{
		for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
		{
			if (SessionSearch->SearchResults[i].Session.OwningUserId != Player->GetPreferredUniqueNetId().GetUniqueNetId())
			{
				SearchResult = SessionSearch->SearchResults[i];
				JoinCertainSession(Player->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, SearchResult);
				break;
			}
		}
	}	
}

void UShootingGameInstance::DestroySessionAndLeaveGame()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
			Sessions->DestroySession(GameSessionName);
		}
	}
}

void UShootingGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	// UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful);

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		return;
	}
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	if (!Sessions.IsValid())
	{
		return;
	}

	Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	if (bWasSuccessful)
	{
		OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
		Sessions->StartSession(SessionName);
	}
}

void UShootingGameInstance::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	// UE_LOG(LogTemp, Warning, TEXT("OnStartSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful);

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		return;
	}
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
	}

	if (bWasSuccessful)
	{
		UGameplayStatics::OpenLevel(GetWorld(), "ThirdPersonExampleMap", true, "listen");
	}
}

void UShootingGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	// UE_LOG(LogTemp, Warning, TEXT("OFindSessionsComplete bSuccess: %d"), bWasSuccessful);

	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		return;
	}
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	if (!Sessions.IsValid())
	{
		return;
	}
	Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

	UE_LOG(LogTemp, Warning, TEXT("Num Search Results: %d"), SessionSearch->SearchResults.Num());

	if (SessionSearch->SearchResults.Num() > 0)
	{
		for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
		{
			// UE_LOG(LogTemp, Warning, TEXT("Session Number: %d | Session Name: %s "), SearchIdx + 1, *(SessionSearch->SearchResults[SearchIdx].Session.OwningUserName));
		}
	}
}

void UShootingGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete %s, %d"), *SessionName.ToString(), static_cast<int32>(Result));

	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		return;
	}
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	if (!Sessions.IsValid())
	{
		return;
	}

	Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	FString TravelURL;
	if (PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL))
	{
		// UE_LOG(LogTemp, Warning, TEXT("Travel URL: %s"), *TravelURL);
		PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
	}
}

void UShootingGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	// UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful);

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		return;
	}
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

	if (Sessions.IsValid())
	{
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		if (bWasSuccessful)
		{
			UGameplayStatics::OpenLevel(GetWorld(), "MainMenuMap", true);
		}
	}
}
