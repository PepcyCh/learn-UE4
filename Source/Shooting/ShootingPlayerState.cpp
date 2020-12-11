// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingPlayerState.h"



#include "ShootingGameInstance.h"
#include "ShootingGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

AShootingPlayerState::AShootingPlayerState()
{
    bReplicates = true;
    bUseCustomPlayerNames = true;
    UE_LOG(LogTemp, Log, TEXT("PlayerState ctor"));
}

void AShootingPlayerState::SetCustomPlayerName_Server_Implementation(const FString& Name)
{
    CustomPlayerName = Name;
}

FString AShootingPlayerState::GetPlayerNameCustom() const
{
    return CustomPlayerName;
}

void AShootingPlayerState::IncreaseKillNumber_Server_Implementation()
{
    ++KillNumber;
}
bool AShootingPlayerState::IncreaseKillNumber_Server_Validate()
{
    AShootingGameState* GameState = Cast<AShootingGameState>(GetWorld()->GetGameState());
    return GameState && GameState->GetCurrentState() == EShootingState::InGame;
}

void AShootingPlayerState::ResetKillNumber_Server_Implementation()
{
    KillNumber = 0;
}

void AShootingPlayerState::IncreaseDeathNumber_Server_Implementation()
{
    ++DeathNumber;
}
bool AShootingPlayerState::IncreaseDeathNumber_Server_Validate()
{
    AShootingGameState* GameState = Cast<AShootingGameState>(GetWorld()->GetGameState());
    return GameState && GameState->GetCurrentState() == EShootingState::InGame;
}

void AShootingPlayerState::ResetDeathNumber_Server_Implementation()
{
    DeathNumber = 0;
}

void AShootingPlayerState::InsertRecordToInstance_Client_Implementation()
{
    UShootingGameInstance* GameInstance = Cast<UShootingGameInstance>(GetGameInstance());
    if (GameInstance)
    {
        GameInstance->InsertGameRecord(CustomPlayerName, KillNumber, DeathNumber);
    }
}


void AShootingPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AShootingPlayerState, CustomPlayerName);
    DOREPLIFETIME(AShootingPlayerState, KillNumber);
    DOREPLIFETIME(AShootingPlayerState, DeathNumber);
}

void AShootingPlayerState::CopyProperties(APlayerState* PlayerState)
{
    Super::CopyProperties(PlayerState);
    AShootingPlayerState* NewPlayerState = Cast<AShootingPlayerState>(PlayerState);
    if (NewPlayerState)
    {
        NewPlayerState->CustomPlayerName = CustomPlayerName;
        NewPlayerState->KillNumber = KillNumber;
        NewPlayerState->DeathNumber = DeathNumber;
    }
}
void AShootingPlayerState::OverrideWith(APlayerState* PlayerState)
{
    Super::CopyProperties(PlayerState);
    AShootingPlayerState* OldPlayerState = Cast<AShootingPlayerState>(PlayerState);
    if (OldPlayerState)
    {
        CustomPlayerName = OldPlayerState->CustomPlayerName;
        KillNumber = OldPlayerState->KillNumber;
        DeathNumber = OldPlayerState->DeathNumber;
    }
}