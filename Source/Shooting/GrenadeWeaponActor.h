// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponActor.h"
#include "GrenadeWeaponActor.generated.h"

UCLASS()
class SHOOTING_API AGrenadeWeaponActor : public AWeaponActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AGrenadeActor> GrenadeClass;

public:
	AGrenadeWeaponActor();
	
	virtual void Fire(ACharacter* Character, const FVector& Start, const FVector& Direction) override;
	UFUNCTION(Server, Reliable)
	void Fire_Server(ACharacter* Character, const FVector& Start, const FVector& Direction) const;
	void Fire_Server_Implementation(ACharacter* Character, const FVector& Start, const FVector& Direction) const;

	virtual bool CanUsedForBlocking() const override { return false; }
};
