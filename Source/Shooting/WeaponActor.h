// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponActor.generated.h"

UCLASS()
class SHOOTING_API AWeaponActor : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	AController* OwnerController;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WeaponName;

	virtual void BeginPlay() override;
	
public:	
	AWeaponActor();

	void SetOwnerController(AController* Controller);

	UFUNCTION()
	virtual void Fire(ACharacter* Character, const FVector& Start, const FVector& Direction) const;

	UFUNCTION()
	virtual bool CanUsedForBlocking() const { return true; };

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	FString GetWeaponName() const { return WeaponName; }
};
