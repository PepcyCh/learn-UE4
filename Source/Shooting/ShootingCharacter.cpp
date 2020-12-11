// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShootingCharacter.h"

#include "Animation/AnimMontage.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GeneratedCodeHelpers.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceDebug.h"
#include "ShootingPlayerState.h"
#include "ShootingPlayerController.h"
#include "ShootingGameMode.h"
#include "TimerManager.h"
#include "WeaponActor.h"
#include "Misc/OutputDeviceNull.h"

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

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AShootingCharacter::OnOverlapBegin);
	// GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AShootingCharacter::OnOverlapEnd);

	bReplicates = true;
}

void AShootingCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShootingCharacter, Health);
	DOREPLIFETIME(AShootingCharacter, bIsDead);
	DOREPLIFETIME(AShootingCharacter, bIsFiring);
	DOREPLIFETIME(AShootingCharacter, bIsBlocking);
}

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

void AShootingCharacter::Tick(float DeltaTime)
{
	UpdateHealthBar();
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
UWidgetComponent* AShootingCharacter::GetFloatingBarComponent() const
{
	TInlineComponentArray<UWidgetComponent*> CompoArray;
	GetComponents(CompoArray);
	for (auto Compo : CompoArray)
	{
		if (Compo->GetFName() == FName("FloatingBar"))
		{
			return Cast<UWidgetComponent>(Compo);
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
				Weapon->SetOwnerController(GetController());
				Weapon->SetOwner(GetController());
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

void AShootingCharacter::ResetHealth_Server_Implementation()
{
	Health = 100;
	bIsDead = false;
}

FString AShootingCharacter::GetWeaponName() const
{
	return Weapon ? Weapon->GetWeaponName() : "No Weapon";
}


void AShootingCharacter::StartGame()
{
	bInGame = true;
	ResetHealth_Server();
	
	GetGunMeshComponent()->SetStaticMesh(nullptr);
	if (Weapon)
	{
		Weapon->Destroy();
	}
	Weapon = nullptr;
}

void AShootingCharacter::EndGame()
{
	bInGame = false;
}

void AShootingCharacter::FireBegin()
{
	if (bIsBlocking || bIsFiring || Weapon == nullptr || !bInGame || Health <= 0)
	{
		return;
	}

	Fire_Server();
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AShootingCharacter::FireEnd, 0.8f);
	const FVector Direction = FollowCamera->GetForwardVector();
	const FVector Start = GetActorLocation() + Direction * 25.0f + FVector(0.0f, 0.0f, 50.0f);
	Weapon->Fire(this, Start, Direction);
}

void AShootingCharacter::FireEnd()
{
	FireEnd_Server();
}

void AShootingCharacter::BlockBegin()
{
	if (bIsBlocking || bIsFiring || !bInGame || Weapon == nullptr || !Weapon->CanUsedForBlocking() || Health <= 0)
	{
		return;
	}
	Block_Server();
	GetWorldTimerManager().SetTimer(BlockTimerHandle, this, &AShootingCharacter::BlockEnd, 2.5f);
}

void AShootingCharacter::BlockEnd()
{
	BlockEnd_Server();
}

float AShootingCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health <= 0)
	{
		return 0.0f;
	}
	if (bIsDead || bIsBlocking)
	{
		DamageAmount = 0.0f;
	}
	if (Health - DamageAmount < 0)
	{
		DamageAmount = Health;
	}
	Health -= DamageAmount;
	if (Health <= 0)
	{
		Health = 0;
		OnDeath(Cast<AShootingCharacter>(EventInstigator->GetPawn()));		
	}
	return DamageAmount;
}

void AShootingCharacter::UpdateHealthBar()
{
	UWidgetComponent* HealthBar = GetFloatingBarComponent();
	if (HealthBar)
	{
		UUserWidget* HealthBarWidget = HealthBar->GetUserWidgetObject();
		if (HealthBarWidget)
		{
			const FString Func = FText::Format(FTextFormat::FromString("UpdateHealthBar {0}"), FMath::Max(0.0f, Health / 100.0f)).ToString();
			FOutputDeviceNull OutputDevice;
			HealthBarWidget->CallFunctionByNameWithArguments(*Func, OutputDevice, this, true);
		}
	}
}

void AShootingCharacter::OnDeath(AShootingCharacter* Killer)
{
	bIsDead = true;
	if (Killer)
	{
		AShootingPlayerState* KillerPlayerState = Cast<AShootingPlayerState>(Killer->GetPlayerState());
		if (KillerPlayerState)
		{
			KillerPlayerState->IncreaseKillNumber_Server();
		}
	}
	AShootingPlayerState* ShootingPlayerState = Cast<AShootingPlayerState>(GetPlayerState());
	if (ShootingPlayerState)
	{
		ShootingPlayerState->IncreaseDeathNumber_Server();
	}

	AShootingGameMode* ShootingGameMode = Cast<AShootingGameMode>(GetWorld()->GetAuthGameMode());
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (ShootingGameMode && PlayerController)
	{
		ShootingGameMode->RestartPlayerDelay(PlayerController, 5.0f);
	}

	PlayDeathMontage_Multicast();
	GetWorldTimerManager().SetTimer(DyingTimerHandle, this, &AShootingCharacter::AfterDeath, 2.5f);
}

void AShootingCharacter::AfterDeath()
{
	// TODO - restart
	AShootingPlayerController* ShootingPlayerController = Cast<AShootingPlayerController>(GetController());
	if (ShootingPlayerController)
	{
		ShootingPlayerController->AddDyingUIToViewport_Client();
	}
	Destroy();
}

void AShootingCharacter::Fire_Server_Implementation()
{
	bIsFiring = true;
	PlayFireMontage_Multicast();
}
void AShootingCharacter::FireEnd_Server_Implementation()
{
	bIsFiring = false;
}

void AShootingCharacter::Block_Server_Implementation()
{
	bIsBlocking = true;
	PlayBlockMontage_Multicast();
}
void AShootingCharacter::BlockEnd_Server_Implementation()
{
	bIsBlocking = false;
	GetGunMeshComponent()->SetRelativeRotation(FRotator(32.799591f, 257.086487f, 155.704193f));
}

void AShootingCharacter::PlayFireMontage_Multicast_Implementation()
{
	if (FireMontage)
	{
		PlayAnimMontage(FireMontage);
	}
}
void AShootingCharacter::PlayBlockMontage_Multicast_Implementation()
{
	GetGunMeshComponent()->SetRelativeRotation(FRotator(-26.968733f, 253.489838f, 64.647484f));
	if (BlockMontage)
	{
		PlayAnimMontage(BlockMontage);
	}
}
void AShootingCharacter::PlayDeathMontage_Multicast_Implementation()
{
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
}

