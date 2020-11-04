// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShootingCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "ShootingPlayerState.h"
#include "ShootingPlayerController.h"
#include "ShootingGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceDebug.h"

//////////////////////////////////////////////////////////////////////////
// AShootingCharacter

AShootingCharacter::AShootingCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AShootingCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// I'm not sure why these settings have on effects in ctor
	// bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	// bUseControllerRotationRoll = true;

	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShootingCharacter::Shoot);
	// PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	// PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShootingCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShootingCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShootingCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShootingCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AShootingCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AShootingCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AShootingCharacter::OnResetVR);
}


void AShootingCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AShootingCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// Jump();
}

void AShootingCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	// StopJumping();
}

void AShootingCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShootingCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShootingCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AShootingCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AShootingCharacter::Shoot()
{
	AShootingPlayerState* ShootingPlayerState = GetPlayerState<AShootingPlayerState>();
	if (ShootingPlayerState == NULL)
	{
		return;
	}

	const FVector Start = GetActorLocation() + FVector(10.0f, 20.0f, 50.0f);
	const FVector Direction = FollowCamera->GetForwardVector();
	const float ShootRayLength = 10000.0f;
	const FVector End = Start + Direction * ShootRayLength;
	// UE_LOG(LogTemp, Warning, TEXT("Trace Ray, Start = %s, End = %s"), *Start.ToString(), *End.ToString());

	// GetWorld()->DebugDrawTraceTag = FName("Bullet");
	FCollisionQueryParams QueryParams(FName("Bullet"), true, this);
	QueryParams.bReturnPhysicalMaterial = false;
	QueryParams.bTraceComplex = true;
	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams);
	AActor* HitActor = Cast<AActor>(Hit.GetActor());
	if (HitActor)
	{
		// UE_LOG(LogTemp, Warning, TEXT("Hit Actor Name = %s"), *HitActor->GetFName().ToString());
		FVector HitLocation = Hit.Location;
		if (HitParticle)
		{
			FTransform ParticleTrans(HitLocation);
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, ParticleTrans);
		}

		if (bInGame && HitActor->GetFName() == FName("Target_BP"))
		{
			FVector TargetLocation = HitActor->GetActorLocation();
			const float Diff = (TargetLocation - HitLocation).Size();
			const float Radius = 80.0f;
			const float RadiusInner = 30.0f;
			uint32 Score = 10;
			if (Diff <= RadiusInner)
			{
				Score = 10;
			}
			else if (Diff >= Radius)
			{
				Score = 0;
			}
			else
			{
				Score = (Radius - Diff) / (Radius - RadiusInner) * 10.0f;
			}
			// UE_LOG(LogTemp, Warning, TEXT("Diff = %f, Score = %d"), Diff, Score);
			ShootingPlayerState->IncreaseScore(Score);
		}
	}
}

void AShootingCharacter::StartTimer()
{
	AShootingPlayerState* ShootingPlayerState = GetPlayerState<AShootingPlayerState>();
	ShootingPlayerState->ResetScore();
	const static float GAME_TIME = 30.0f;
	GetWorldTimerManager().SetTimer(ShootTimerHandle, this, &AShootingCharacter::OnTimerEnd, GAME_TIME);
	bInGame = true;
}

void AShootingCharacter::OnTimerEnd()
{
	bInGame = false;
	GetWorldTimerManager().ClearTimer(ShootTimerHandle);

	AShootingPlayerState* ShootingPlayerState = GetPlayerState<AShootingPlayerState>();
	Cast<UShootingGameInstance>(GetGameInstance())->SetPlayerScore(ShootingPlayerState->GetShootingScore());

	AShootingPlayerController* PlayerController = Cast<AShootingPlayerController>(GetController());
	if (!PauseMenu)
	{
		FStringClassReference PauseMenuRef(TEXT("/Game/ThirdPersonCPP/Blueprints/PauseMenuUI.PauseMenuUI_C"));
		UClass* PauseMenuClass = PauseMenuRef.TryLoadClass<UUserWidget>();
		if (!PauseMenuClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Can't load widget"));
			return;
		}
		PauseMenu = CreateWidget<UUserWidget>(PlayerController, PauseMenuClass);
	}

	FOutputDeviceDebug DebugDevice;
	PauseMenu->CallFunctionByNameWithArguments(TEXT("SetValues"), DebugDevice, this, true);

	PauseMenu->AddToViewport();
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetWidgetToFocus(PauseMenu->TakeWidget());
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;

}

int32 AShootingCharacter::GetTimeRemaining() const
{
	if (!bInGame)
	{
		return -1;
	}
	return (int32) GetWorldTimerManager().GetTimerRemaining(ShootTimerHandle);
}
