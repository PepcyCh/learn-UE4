// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "WeaponActor.h"
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

	UPROPERTY(VisibleAnywhere)
	bool bInGame = false;

	UPROPERTY(VisibleAnywhere)
	FTimerHandle ShootTimerHandle;
	UPROPERTY(VisibleAnywhere)
	FTimerHandle BlockTimerHandle;
	UPROPERTY(VisibleAnywhere)
	FTimerHandle FireTimerHandle;

	UPROPERTY(VisibleAnywhere)
	UUserWidget* PauseMenu = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AWeaponActor* Weapon = nullptr;

	UFUNCTION()
	UStaticMeshComponent* GetGunMeshComponent() const;

public:
	AShootingCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsBlocking = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsFiring = false;

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

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

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

	UFUNCTION()
	void StartTimer();
	UFUNCTION()
	void OnTimerEnd();
	UFUNCTION()
	int32 GetTimeRemaining() const;
};

