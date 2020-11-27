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
#include "WeaponActor.h"
#include "GrenadeActor.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceDebug.h"
#include "TimerManager.h"
#include "Engine/World.h"

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

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AShootingCharacter::OnOverlapBegin);
	// GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AShootingCharacter::OnOverlapEnd);
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
	// PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShootingCharacter::Shoot);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShootingCharacter::FireBegin);
	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &AShootingCharacter::BlockBegin);

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
}

void AShootingCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{	
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
	if (Controller && Value != 0.0f)
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
	if (Controller && Value != 0.0f)
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

UStaticMeshComponent* AShootingCharacter::GetGunMeshComponent() const
{
	TInlineComponentArray<UStaticMeshComponent*> CompoArray;
	GetComponents(CompoArray);
	for (auto Compo : CompoArray)
	{
		if (Compo->GetFName() == FName("GunMesh"))
		{
			return Cast<UStaticMeshComponent>(Compo);
		}
	}
	return nullptr;
}


void AShootingCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		AWeaponActor* WeaponActor = Cast<AWeaponActor>(OtherActor);
		if (WeaponActor)
		{
			// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Overlap Weapon Actor"));
			UStaticMeshComponent* WeaponMeshCompo = Cast<UStaticMeshComponent>(OtherComp);
			if (WeaponMeshCompo && WeaponMeshCompo->IsVisible())
			{
				GetGunMeshComponent()->SetStaticMesh(WeaponMeshCompo->GetStaticMesh());
				if (Weapon)
				{
					Weapon->Destroy();
				}
				Weapon = WeaponActor;
				Cast<AShootingPlayerState>(GetPlayerState())->SetWeaponName(Weapon->GetWeaponName());
				WeaponMeshCompo->SetVisibility(false);
			}
		}
	}
}

// void AShootingCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
// 	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
// {
// 	if (OtherActor && (OtherActor != this) && OtherComp)
// 	{
// 		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString("On Overlap Begin, Actor Name = ") + OtherActor->GetName());
// 	}
// }

void AShootingCharacter::FireBegin()
{
	if (bIsBlocking || bIsFiring || Weapon == nullptr || !bInGame)
	{
		return;
	}

	bIsFiring = true;
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AShootingCharacter::FireEnd, 1.0f);
	const FVector Direction = FollowCamera->GetForwardVector();
	const FVector Start = GetActorLocation() + Direction * 25.0f + FVector(0.0f, 0.0f, 50.0f);
	Weapon->Fire(this, Start, Direction);
}

void AShootingCharacter::FireEnd()
{
	bIsFiring = false;
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
}

void AShootingCharacter::StartTimer()
{
	AShootingPlayerState* ShootingPlayerState = GetPlayerState<AShootingPlayerState>();
	ShootingPlayerState->ResetScore();
	const static float GameTime = 60.0f;
	GetWorldTimerManager().SetTimer(ShootTimerHandle, this, &AShootingCharacter::OnTimerEnd, GameTime);
	bInGame = true;
	GetGunMeshComponent()->SetStaticMesh(nullptr);
	if (Weapon)
	{
		Weapon->Destroy();
	}
	Weapon = nullptr;
	Cast<AShootingPlayerState>(GetPlayerState())->SetWeaponName("No Weapon");
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
		const FStringClassReference PauseMenuRef(TEXT("/Game/ThirdPersonCPP/Blueprints/UI/PauseMenuUI.PauseMenuUI_C"));
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
	// InputMode.SetWidgetToFocus(PauseMenu->TakeWidget());
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}

int32 AShootingCharacter::GetTimeRemaining() const
{
	if (!bInGame)
	{
		return -1;
	}
	return static_cast<int32>(GetWorldTimerManager().GetTimerRemaining(ShootTimerHandle));
}

void AShootingCharacter::BlockBegin()
{
	if (bIsBlocking || bIsFiring || !bInGame || !Weapon->CanUsedForBlocking())
	{
		return;
	}
	bIsBlocking = true;
	GetGunMeshComponent()->SetRelativeRotation(FRotator(-26.968733f, 253.489838f, 64.647484f));
	GetWorldTimerManager().SetTimer(BlockTimerHandle, this, &AShootingCharacter::BlockEnd, 2.5f);
}

void AShootingCharacter::BlockEnd()
{
	bIsBlocking = false;
	GetGunMeshComponent()->SetRelativeRotation(FRotator(32.799591f, 257.086487f, 155.704193f));
	GetWorldTimerManager().ClearTimer(BlockTimerHandle);
}
