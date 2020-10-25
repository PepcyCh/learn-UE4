// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShootingGameMode.h"
#include "ShootingCharacter.h"
#include "UObject/ConstructorHelpers.h"

#include "ShootingGameState.h"
#include "ShootingPlayerState.h"
#include "ShootingPlayerController.h"
#include "ShootingHUD.h"
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
	// if (GetWorld()->IsServer())
	// {
	//	return;
	// }
	// UE_LOG(LogTemp, Warning, TEXT("GM::RestartLogin, PC ?= %d"), !!NewPlayer);
	AShootingPlayerController* Controller = Cast<AShootingPlayerController>(NewPlayer);
	AShootingPlayerState* PlayerState = Controller->GetPlayerState<AShootingPlayerState>();

	APawn* OldPawn = Controller->GetPawn();
	if (OldPawn)
	{
		OldPawn->Destroy();
	}

	int32 PlayerId = PlayerState->Id;
	TArray<AActor*> FoundPlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundPlayerStarts);
	if (FoundPlayerStarts.Num() == 0)
	{
		return;
	}
	APlayerStart* PlayerStart = Cast<APlayerStart>(FoundPlayerStarts[0]);

	FRotator Rotator = PlayerStart->GetActorRotation();
	FVector Location = PlayerStart->GetActorLocation();
	FTransform Transform(Rotator, Location, FVector(1.0f, 1.0f, 1.0f));
	APawn* Pawn = SpawnDefaultPawnAtTransform(Controller, Transform);
	Controller->SetPawn(Pawn);
	Controller->SetupInputComponent();
}

void AShootingGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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