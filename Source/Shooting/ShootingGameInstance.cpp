// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingGameInstance.h"

#include "ShootingPlayerController.h"
#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "ShootingCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/GameModeBase.h"

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

void UShootingGameInstance::InsertGameRecord(const FString& Name, int32 KilledCount, int32 DeathCount)
{
	const FPlayerScorePair Item { Name, KilledCount, DeathCount };
	RankList.Add(Item);
	for (int32 i = RankList.Num() - 1; i > 0; i--)
	{
		RankList[i] = RankList[i - 1];
	}
	RankList[0] = Item;
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
	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && UserId.IsValid())
		{
			SessionSettings = MakeShareable(new FOnlineSessionSettings());
			SessionSettings->bIsLANMatch = bIsLAN;
			SessionSettings->bUsesPresence = bIsPresence;
			SessionSettings->NumPublicConnections = MaxNumPlayers;
			SessionSettings->NumPrivateConnections = 0;
			SessionSettings->bAllowInvites = true;
			SessionSettings->bAllowJoinInProgress = true;
			SessionSettings->bShouldAdvertise = true;
			SessionSettings->bAllowJoinViaPresence = true;
			SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
			SessionSettings->Set(SETTING_MAPNAME, FString("MainMenuMap"), EOnlineDataAdvertisementType::ViaOnlineService);

			OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
			return Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
		}
		else
		{
			OnCreateSessionComplete(SessionName, false);
		}
	}

	return false;
}

void UShootingGameInstance::FindSessions(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && UserId.IsValid())
		{
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = bIsLAN;
			SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);

			const TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();
			OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
			Sessions->FindSessions(*UserId, SearchSettingsRef);
		}
	}
	else
	{
		OnFindSessionsComplete(false);
	}
}

bool UShootingGameInstance::JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId().IsValid())
		{
			OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
			return Sessions->JoinSession(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, SearchResult);
		}
	}

	return false;
}

void UShootingGameInstance::HostOnlineGame(bool bIsLan, bool bIsPresence, int32 MaxNumPlayers)
{
	ULocalPlayer* Player = GetFirstGamePlayer();
	UE_LOG(LogTemp, Log, TEXT("Host Online Game, pid = %s"), *Player->GetPreferredUniqueNetId().GetUniqueNetId()->ToString());
	OnlineGamePlayerLimit = MaxNumPlayers;
	HostSession(Player->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, bIsLan, bIsPresence, MaxNumPlayers);
}

void UShootingGameInstance::StartOnlineGame()
{
	UE_LOG(LogTemp, Log, TEXT("Start Online Game"));

	if (GetWorld()->GetAuthGameMode())
	{
		OnlineGamePlayerNumber = GetWorld()->GetAuthGameMode()->GetNumPlayers();
		UE_LOG(LogTemp, Log, TEXT("Start Online Game, Player Number = %d"), OnlineGamePlayerNumber);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AShootingPlayerController* PlayerController = Cast<AShootingPlayerController>(*It);
		if (PlayerController)
		{
			PlayerController->StartOnlineGame_Client();
		}
	}

	const FString GameMapUrl = "/Game/Maps/GameMap";
	GetWorld()->ServerTravel(GameMapUrl);
}

void UShootingGameInstance::FindOnlineGames(bool bIsLan, bool bIsPresence)
{
	ULocalPlayer* Player = GetFirstGamePlayer();
	UE_LOG(LogTemp, Log, TEXT("Find Online Game, pid = %s"), *Player->GetPreferredUniqueNetId().GetUniqueNetId()->ToString());
	FindSessions(Player->GetPreferredUniqueNetId().GetUniqueNetId(), bIsLan, bIsPresence);
}

void UShootingGameInstance::JoinOnlineGame(const FOnlineSessionSearchResult& SearchResult)
{
	ULocalPlayer* Player = GetFirstGamePlayer();
	JoinSession(Player, SearchResult);

	// UE_LOG(LogTemp, Log, TEXT("[Join Online Game] Num Search Results: %d"), SessionSearch->SearchResults.Num());
	// if (SessionSearch->SearchResults.Num() > 0)
	// {
	// 	for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
	// 	{
	// 		if (SessionSearch->SearchResults[i].Session.OwningUserId != Player->GetPreferredUniqueNetId().GetUniqueNetId())
	// 		{
	// 			const FOnlineSessionSearchResult SearchResult = SessionSearch->SearchResults[i];
	// 			JoinSession(Player, SearchResult);
	// 			break;
	// 		}
	// 	}
	// }
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
	UE_LOG(LogTemp, Log, TEXT("OnCreateSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful);

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	}
}

void UShootingGameInstance::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("OnStartSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful);

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}
}

void UShootingGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("OFindSessionsComplete bSuccess: %d"), bWasSuccessful);

	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

			UE_LOG(LogTemp, Log, TEXT("[Find Online Game] Num Search Results: %d"), SessionSearch->SearchResults.Num());
			for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
			{
				const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[i];
				UE_LOG(LogTemp, Log, TEXT(" Search Result: Owner: %s"), *SearchResult.Session.OwningUserId->ToString());
				DumpSession(&SearchResult.Session);
				JoinOnlineGame(SearchResult);
			}
		}
	}
}

void UShootingGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Log, TEXT("OnJoinSessionComplete %s, Res = %d (0 is OK)"), *SessionName.ToString(), static_cast<int32>(Result));

	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
		}
	}
	
	if (Result == EOnJoinSessionCompleteResult::Type::Success)
	{
		const TArray<ULocalPlayer*> LocalPlayersTemp = GetLocalPlayers();
		int32 Id = -1;
		for (int32 i = 0; i < LocalPlayersTemp.Num(); i++)
		{
			if (GetPrimaryPlayerController()->GetLocalPlayer() == LocalPlayersTemp[i])
			{
				Id = i;
				break;
			}
		}
		if (Id >= 0)
		{
			ClientTravelToSession(Id, NAME_GameSession);
		}
	}
}

void UShootingGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful);

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		SessionSettings = nullptr;
	}
}
