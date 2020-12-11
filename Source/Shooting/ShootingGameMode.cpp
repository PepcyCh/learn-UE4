// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShootingGameMode.h"

#include "EngineUtils.h"
#include "ShootingCharacter.h"
#include "ShootingGameInstance.h"
#include "UObject/ConstructorHelpers.h"

#include "TimerManager.h"
#include "ShootingGameState.h"
#include "ShootingPlayerState.h"
#include "ShootingPlayerController.h"
#include "WeaponActor.h"
#include "GameFramework/PlayerStart.h"

AShootingGameMode::AShootingGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	GameStateClass = AShootingGameState::StaticClass();
	PlayerStateClass = AShootingPlayerState::StaticClass();
	PlayerControllerClass = AShootingPlayerController::StaticClass();
}

void AShootingGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AShootingGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (GetCurrentMapName() == "WaitingMap")
	{
		AShootingPlayerController* ShootingPlayerController = Cast<AShootingPlayerController>(NewPlayer);
		if (ShootingPlayerController)
		{
			ShootingPlayerController->AddWaitingUIToViewport_Client();
		}

		++LoginPlayerCount;
		UShootingGameInstance* ShootingGameInstance = Cast<UShootingGameInstance>(GetGameInstance());
		const int32 NumMax = ShootingGameInstance ? ShootingGameInstance->GetOnlineGamePlayerLimit() : 0;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AShootingPlayerController* PlayerController = Cast<AShootingPlayerController>(*It);
			if (PlayerController)
			{
				PlayerController->UpdateWaitingUI_Client(LoginPlayerCount, NumMax);
			}
		}
	}
}

void AShootingGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	AShootingPlayerController* Controller = Cast<AShootingPlayerController>(NewPlayer);

	APawn* Pawn = Controller->GetPawn();
	check(Pawn);
	Controller->SetPawn(Pawn);
	Controller->SetupInputComponent();
	++SpawnedPlayerCount;
	if (GetCurrentMapName() == "GameMap")
	{
		if (Controller)
		{
			Controller->StartGame_Client();
		}
		UShootingGameInstance* ShootingGameInstance = Cast<UShootingGameInstance>(GetGameInstance());
		if (ShootingGameInstance && SpawnedPlayerCount == ShootingGameInstance->GetOnlineGamePlayerNumber())
		{
			StartGameTimer();
		}
	}
}
void AShootingGameMode::RestartPlayerDelay(APlayerController* NewPlayer, float DelayTime)
{
	GetWorldTimerManager().SetTimer(RestartTimerHandle, [this, NewPlayer]()
	{
		RestartPlayer(NewPlayer);
	}, DelayTime, false);
}

AActor* AShootingGameMode::ChoosePlayerStart_Implementation(AController* Controller)
{
	int TotalCount = 0;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		++TotalCount;
	}
	if (TotalCount == 0)
	{
		return Super::ChoosePlayerStart_Implementation(Controller);
	}
	
	int Count = SpawnedPlayerCount % TotalCount;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (Count == 0)
		{
			return *It;
		}
		--Count;
	}
	return Super::ChoosePlayerStart_Implementation(Controller);
}

void AShootingGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AccumulatedTime += DeltaSeconds;
	const static float SpawnWeaponDelta = 2.5f;
	if (AccumulatedTime >= SpawnWeaponDelta)
	{
		AccumulatedTime -= SpawnWeaponDelta;
		SpawnRandomWeapon();
	}
}

void AShootingGameMode::SpawnRandomWeapon() const
{
	if (Weapons.Num() == 0)
	{
		return;
	}
	
	const static uint32 MaxWeaponCount = 15;
	int32 FoundWeaponCount = 0;
	for (TActorIterator<AWeaponActor> It(GetWorld()); It; ++It)
	{
		if (It->GetRootComponent() && It->GetRootComponent()->IsVisible())
		{
			++FoundWeaponCount;
		}
	}
	if (FoundWeaponCount >= MaxWeaponCount)
	{
		return;
	}

	const static float PosXMax = 700.0f;
	const static float PosXMin = -1600.0f;
	const static float PosYMax = 1000.0f;
	const static float PosYMin = -1000.0f;
	const static float PosZMax = 150.0f;
	const static float PosZMin = 150.0f;

	const float PosX = FMath::RandRange(PosXMin, PosXMax);
	const float PosY = FMath::RandRange(PosYMin, PosYMax);
	const float PosZ = FMath::RandRange(PosZMin, PosZMax);
	const float Roll = FMath::RandRange(0.0f, 360.0f);
	const FTransform WeaponTransform(FRotator(90.0f, 0.0f, Roll), FVector(PosX, PosY, PosZ));

	GetWorld()->SpawnActor<AWeaponActor>(Weapons[FMath::RandRange(0, Weapons.Num() - 1)], WeaponTransform);
}

FString AShootingGameMode::GetCurrentMapName()
{
	UWorld* World = GetWorld();
	return  World->GetMapName().Mid(World->StreamingLevelsPrefix.Len());
}

void AShootingGameMode::StartGameTimer()
{
	GetWorldTimerManager().SetTimer(GameTimerHandle, this, &AShootingGameMode::OnGameEnd, GameTime);
	Cast<AShootingGameState>(GameState)->ChangeToState(EShootingState::InGame);
}
void AShootingGameMode::OnGameEnd() const
{
	FString NameStr;
	FString KilledStr;
	FString DeathStr;
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		AShootingPlayerState* ShootingPlayerState = Cast<AShootingPlayerState>(PlayerState);
		if (ShootingPlayerState)
		{
			NameStr += ShootingPlayerState->GetPlayerName() + "\n";
			KilledStr += FString::FromInt(ShootingPlayerState->GetKillNumber()) + "\n";
			DeathStr += FString::FromInt(ShootingPlayerState->GetDeathNumber()) + "\n";
			ShootingPlayerState->InsertRecordToInstance_Client();
		}
	}
	
	Cast<AShootingGameState>(GameState)->ChangeToState(EShootingState::End);
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AShootingPlayerController *PlayerController = Cast<AShootingPlayerController>(*It);
		if (PlayerController)
		{
			PlayerController->EndOnlineGame_Client(NameStr, KilledStr, DeathStr);
		}
	}
}

