// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "WeaponActor.h"
#include "GunWeaponActor.generated.h"

UCLASS()
class SHOOTING_API AGunWeaponActor : public AWeaponActor
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UParticleSystem* HitParticle;

    UPROPERTY()
    class ASoundHelper* SoundHelper;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ASoundHelper> SoundHelperClass;

    void HitTarget(ACharacter* Character, const FHitResult& Hit) const;

    UFUNCTION(Server, Reliable)
    void HitCharacter_Server(ACharacter* Character, const FHitResult& Hit, int32 Damage) const;
    void HitCharacter_Server_Implementation(ACharacter* Character, const FHitResult& Hit, int32 Damage) const;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void BeginPlay() override;
    
    virtual void Fire(ACharacter* Character, const FVector& Start, const FVector& Direction) const override;
};
