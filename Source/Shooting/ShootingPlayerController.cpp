// Fill out your copyright notice in the Description page of Project Settings.

#include "ShootingPlayerController.h"

#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "ShootingCharacter.h"
#include "ShootingGameInstance.h"
#include "ShootingPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceDebug.h"
#include "Misc/OutputDeviceNull.h"

void AShootingPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    Possess(GetPawn<AShootingCharacter>());
}

void AShootingPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (MainGameUI)
    {
        UpdateMainGameUI();
    }
}

// Part of codes are from Shooter Game
void AShootingPlayerController::StartOnlineGame_Client_Implementation()
{
    if (!IsPrimaryPlayer())
        return;

    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());
    
    AShootingPlayerState* ShootingPlayerState = Cast<AShootingPlayerState>(PlayerState);
    if (ShootingPlayerState)
    {
        UShootingGameInstance* GameInstance = Cast<UShootingGameInstance>(GetGameInstance());
        if (GameInstance)
        {
            ShootingPlayerState->SetCustomPlayerName_Server(GameInstance->GetPlayerName());
        }
        
        IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
        if (OnlineSub)
        {
            IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
            if (Sessions.IsValid() && (Sessions->GetNamedSession(ShootingPlayerState->SessionName) != nullptr))
            {
                UE_LOG(LogTemp, Log, TEXT("Starting session %s on client"), *ShootingPlayerState->SessionName.ToString() );
                Sessions->StartSession(ShootingPlayerState->SessionName);
            }
        }
    }
    else
    {
        GetWorld()->GetTimerManager().SetTimer(RetryTimerHandle, this, &AShootingPlayerController::StartOnlineGame_Client_Implementation, 0.2f);
    }
}
void AShootingPlayerController::StartGame_Client_Implementation()
{
    AShootingCharacter* ShootingCharacter = Cast<AShootingCharacter>(GetPawn());
    if (!ShootingCharacter)
    {
        GetWorldTimerManager().SetTimer(RetryTimerHandle, this, &AShootingPlayerController::StartGame_Client_Implementation, 0.2f, false);
        return;
    }
    ShootingCharacter->StartGame();

    if (WaitingUI)
    {
        WaitingUI->RemoveFromViewport();
    }
    if (DyingUI)
    {
        DyingUI->RemoveFromViewport();
    }
    if (!MainGameUI && MainGameUIClass)
    {
        MainGameUI = CreateWidget<UUserWidget>(this, MainGameUIClass);
    }
    if (MainGameUI)
    {
        MainGameUI->AddToViewport();
    }

    const float GameTime = 90.0f;
    if (GetRemainingTime() < 0)
    {
        GetWorldTimerManager().SetTimer(GameTimerHandle, [](){}, GameTime, false);
    }
}

void AShootingPlayerController::EndOnlineGame_Client_Implementation(const FString& NameStr, const FString& KilledStr, const FString& DeathStr)
{
    AShootingCharacter* ShootingCharacter = Cast<AShootingCharacter>(GetPawn());
    if (ShootingCharacter)
    {
        ShootingCharacter->EndGame();
    }

    if (MainGameUI)
    {
        MainGameUI->RemoveFromViewport();
    }

    if (!EndingUI && EndingUIClass)
    {
        EndingUI = CreateWidget<UUserWidget>(this, EndingUIClass);
    }
    if (EndingUI)
    {
        FOutputDeviceNull OutputDevice;
        const FString FuncName = "UpdateListName " + NameStr;
        EndingUI->CallFunctionByNameWithArguments(*FuncName, OutputDevice, this, true);
        const FString FuncKill = "UpdateListKill " + KilledStr;
        EndingUI->CallFunctionByNameWithArguments(*FuncKill, OutputDevice, this, true);
        const FString FuncDeath = "UpdateListDeath " + DeathStr;
        EndingUI->CallFunctionByNameWithArguments(*FuncDeath, OutputDevice, this, true);
        EndingUI->AddToViewport();
    }

    FInputModeUIOnly InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
    bShowMouseCursor = true;
}

// Part of codes are from Shooter Game
void AShootingPlayerController::LeaveOnlineGame_Client_Implementation()
{
    if (!IsPrimaryPlayer())
        return;

    AShootingPlayerState* ShooterPlayerState = Cast<AShootingPlayerState>(PlayerState);
    if (ShooterPlayerState)
    {
        IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
        if (OnlineSub)
        {
            IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
            if (Sessions.IsValid() && (Sessions->GetNamedSession(ShooterPlayerState->SessionName) != nullptr))
            {
                UE_LOG(LogOnline, Log, TEXT("Ending session %s on client"), *ShooterPlayerState->SessionName.ToString() );
                Sessions->EndSession(ShooterPlayerState->SessionName);
            }
        }
    }

    UGameplayStatics::OpenLevel(GetWorld(), "/Game/Maps/MainMenu");
}

int32 AShootingPlayerController::GetRemainingTime() const
{
    return static_cast<int32>(GetWorldTimerManager().GetTimerRemaining(GameTimerHandle));
}

void AShootingPlayerController::SetPlayerName(const FString& Name)
{
    AShootingPlayerState* ShootingPlayerState = GetPlayerState<AShootingPlayerState>();
    if (ShootingPlayerState)
    {
        ShootingPlayerState->SetCustomPlayerName_Server(Name);
    }
}

void AShootingPlayerController::AddWaitingUIToViewport_Client_Implementation()
{
    if (!WaitingUI && WaitingUIClass)
    {
        WaitingUI = CreateWidget<UUserWidget>(this, WaitingUIClass);
    }
    if (WaitingUI)
    {
        WaitingUI->AddToViewport();
        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
}
void AShootingPlayerController::UpdateWaitingUI_Client_Implementation(int32 NumCurr, int32 NumMax)
{
    if (WaitingUI)
    {
        const FString Func = FText::Format(FTextFormat::FromString("UpdateNumber {0} {1}"), NumCurr, NumMax).ToString();
        FOutputDeviceNull OutputDevice;
        WaitingUI->CallFunctionByNameWithArguments(*Func, OutputDevice, this, true);
    }
}

void AShootingPlayerController::AddDyingUIToViewport_Client_Implementation()
{
    if (!DyingUI && DyingUIClass)
    {
        DyingUI = CreateWidget<UUserWidget>(this, DyingUIClass);
    }
    if (DyingUI)
    {
        DyingUI->AddToViewport();
    }
}

void AShootingPlayerController::UpdateMainGameUI()
{
    AGameStateBase* GameState = GetWorld()->GetGameState();
    TArray<APlayerState*> PlayerArray;
    if (GameState)
    {
        PlayerArray = GameState->PlayerArray;
    }
    TArray<AShootingPlayerState*> ShootingPlayerArray;
    for (APlayerState* PlayerState_ : PlayerArray)
    {
        AShootingPlayerState* ShootingPlayerState = Cast<AShootingPlayerState>(PlayerState_);
        if (ShootingPlayerState)
        {
            ShootingPlayerArray.Add(ShootingPlayerState);
        }
    }

    FString StatesStr;
    { // current player
        AShootingPlayerState* ShootingPlayerState = Cast<AShootingPlayerState>(PlayerState);
        AShootingCharacter* ShootingCharacter = Cast<AShootingCharacter>(GetPawn());
        
        FString PlayerName = "";
        if (ShootingPlayerState)
        {
            PlayerName = ShootingPlayerState->GetPlayerName();
        }
        
        int32 Health = 0;
        FString WeaponName = "";
        if (ShootingCharacter)
        {
            Health = ShootingCharacter->GetHealth();
            WeaponName = ShootingCharacter->GetWeaponName();
        }

        StatesStr = "Current Player - " + PlayerName + "\nHealth - " + FString::FromInt(Health) + "\nWeapon - " + WeaponName + "\n";
    }
    for (AShootingPlayerState* ShootingPlayerState : ShootingPlayerArray)
    {
        const FString PlayerName = ShootingPlayerState->GetPlayerName();
        const int32 KillNumber = ShootingPlayerState->GetKillNumber();
        const int32 DeathNumber = ShootingPlayerState->GetDeathNumber();
        StatesStr += "- " + PlayerName + ": Kill - " + FString::FromInt(KillNumber) + "  Death - " + FString::FromInt(DeathNumber) + "\n";
    }

    int32 TimeMin = 0;
    int32 TimeSec = 0;
    { // timer
        const int32 Time = GetRemainingTime();
        if (Time >= 0)
        {
            TimeMin = Time / 60;
            TimeSec = Time % 60;
        }
    }

    FOutputDeviceNull OutputDevice;

    const FString FuncStates = "UpdateStates " + StatesStr;
    MainGameUI->CallFunctionByNameWithArguments(*FuncStates, OutputDevice, this, true);
    const FString FuncTime = "UpdateTime " + FString::FromInt(TimeMin) + " " + FString::FromInt(TimeSec);
    MainGameUI->CallFunctionByNameWithArguments(*FuncTime, OutputDevice, this, true);
}

