// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "WeaponActor.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "ShootingCharacter.generated.h"

UCLASS(config=Game)
class AShootingCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY()
	FTimerHandle BlockTimerHandle;
	UPROPERTY()
	FTimerHandle FireTimerHandle;
	UPROPERTY()
	FTimerHandle DyingTimerHandle;

protected:
	UPROPERTY(VisibleAnywhere, Replicated)
	int Health;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AWeaponActor* Weapon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* FireMontage = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UAnimMontage* BlockMontage = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* DeathMontage = nullptr;

	UFUNCTION()
	UStaticMeshComponent* GetGunMeshComponent() const;
	UFUNCTION()
	UWidgetComponent* GetFloatingBarComponent() const;

	void UpdateHealthBar();

	UFUNCTION(NetMulticast, Reliable)
    void PlayFireMontage_Multicast();
	UFUNCTION(NetMulticast, Reliable)
    void PlayBlockMontage_Multicast();
	UFUNCTION(NetMulticast, Reliable)
	void PlayDeathMontage_Multicast();

	UFUNCTION(Server, Reliable)
	void Fire_Server();
	void Fire_Server_Implementation();
	UFUNCTION(Server, Reliable)
	void FireEnd_Server();
	void FireEnd_Server_Implementation();
	UFUNCTION(Server, Reliable)
	void Block_Server();
	void Block_Server_Implementation();
	UFUNCTION(Server, Reliable)
    void BlockEnd_Server();
	void BlockEnd_Server_Implementation();

public:
	AShootingCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadOnly)
	bool bIsBlocking = false;
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadOnly)
	bool bIsFiring = false;
	UPROPERTY(VisibleAnywhere)
	bool bInGame = false;
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadOnly)
	bool bIsDead = false;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(Server, Reliable)
	void ResetHealth_Server();
	void ResetHealth_Server_Implementation();
	int32 GetHealth() const { return Health; }

	FString GetWeaponName() const;

	UFUNCTION(BlueprintCallable, Category = "Character")
	bool GetIsDead() const { return bIsDead; }

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// UFUNCTION()
	// void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Character")
    void FireBegin();
	UFUNCTION(BlueprintCallable, Category = "Character")
	void FireEnd();

	UFUNCTION(BlueprintCallable, Category = "Character")
	void BlockBegin();
	UFUNCTION(BlueprintCallable, Category = "Character")
	void BlockEnd();

	virtual float TakeDamage(float DamageAmount, const struct FDamageEvent& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	void OnDeath(AShootingCharacter* Killer);
	UFUNCTION()
	void AfterDeath();

	void StartGame();
	void EndGame();
};

