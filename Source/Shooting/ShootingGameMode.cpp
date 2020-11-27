// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShootingGameMode.h"
#include "ShootingCharacter.h"
#include "UObject/ConstructorHelpers.h"

#include "ShootingGameInstance.h"
#include "ShootingGameState.h"
#include "ShootingPlayerState.h"
#include "ShootingPlayerController.h"
#include "ShootingHUD.h"
#include "WeaponActor.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AShootingGameMode::AShootingGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	// UE_LOG(LogTemp, Warning, TEXT("GM::ctor, DP ?= %d"), !!DefaultPawnClass);

	GameStateClass = AShootingGameState::StaticClass();
	PlayerStateClass = AShootingPlayerState::StaticClass();
	PlayerControllerClass = AShootingPlayerController::StaticClass();
	HUDClass = AShootingHUD::StaticClass();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
	for (AActor* Actor : FoundActors)
	{
		if (Actor->GetFName() == FName("Target_BP"))
		{
			Target = Actor;
			break;
		}
	}
	// UE_LOG(LogTemp, Warning, TEXT("GM ctor"));
}

void AShootingGameMode::PostLogin(APlayerController* NewPlayer)
{
	// UE_LOG(LogTemp, Warning, TEXT("GM::PostLogin, PC ?= %d"), !!NewPlayer);
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	AShootingPlayerController* Controller = Cast<AShootingPlayerController>(NewPlayer);
	AShootingPlayerState* PlayerState = Controller->GetPlayerState<AShootingPlayerState>();

	PlayerState->Id = PlayerCount;
	++PlayerCount;
	// UE_LOG(LogTemp, Warning, TEXT("PlayerId = %d"), PlayerState->Id);
}

void AShootingGameMode::RestartPlayer(AController* NewPlayer)
{
	// UE_LOG(LogTemp, Warning, TEXT("GM::RestartLogin, PC ?= %d"), !!NewPlayer);
	AShootingPlayerController* Controller = Cast<AShootingPlayerController>(NewPlayer);
	AShootingPlayerState* PlayerState = Controller->GetPlayerState<AShootingPlayerState>();

	APawn* OldPawn = Controller->GetPawn();
	if (OldPawn)
	{
		OldPawn->Destroy();
	}

	TArray<AActor*> FoundPlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundPlayerStarts);
	if (FoundPlayerStarts.Num() == 0)
	{
		return;
	}
	APlayerStart* PlayerStart = Cast<APlayerStart>(FoundPlayerStarts[0]);

	const FRotator Rotator = PlayerStart->GetActorRotation();
	const FVector Location = PlayerStart->GetActorLocation();
	const FTransform Transform(Rotator, Location);
	APawn* Pawn = SpawnDefaultPawnAtTransform(Controller, Transform);
	Controller->SetPawn(Pawn);
	Controller->SetupInputComponent();
	Controller->StartPlayerTimer();
	PlayerState->SetPlayerName(Cast<UShootingGameInstance>(GetGameInstance())->GetPlayerName());
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

	if (!Target)
	{
		return;
	}

	const float TargetMoveBound = 250.0f;
	const float TargetMoveSpeed = 40.0f;
	FVector TargetLocation = Target->GetActorLocation();

	if (TargetLocation.Y <= -TargetMoveBound)
	{
		TargetMoveDirection = 1;
	}
	else if (TargetLocation.Y >= TargetMoveBound)
	{
		TargetMoveDirection = -1;
	}
	TargetLocation.Y += DeltaSeconds * TargetMoveSpeed * TargetMoveDirection;
	Target->SetActorLocation(TargetLocation);
}

void AShootingGameMode::SpawnRandomWeapon() const
{
	if (Weapons.Num() == 0)
	{
		return;
	}
	
	const static uint32 MaxWeaponCount = 6;
	TArray<AActor*> FoundWeaponActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWeaponActor::StaticClass(), FoundWeaponActors);
	int32 FoundWeaponCount = 0;
	for (const AActor* Actor : FoundWeaponActors)
	{
		if (Actor->GetRootComponent() && Actor->GetRootComponent()->IsVisible())
		{
			++FoundWeaponCount;
		}		
	}
	if (FoundWeaponCount >= MaxWeaponCount)
	{
		return;
	}

	const static float PosXMax = -500.0f;
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
