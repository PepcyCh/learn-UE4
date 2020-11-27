// Fill out your copyright notice in the Description page of Project Settings.


#include "GunWeaponActor.h"

#include "ShootingCharacter.h"
#include "ShootingPlayerState.h"
#include "SoundHelper.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

void AGunWeaponActor::BeginPlay()
{
    if (SoundHelperClass)
    {
        SoundHelper = GetWorld()->SpawnActor<ASoundHelper>(SoundHelperClass, FVector(), FRotator());
    }
}

void AGunWeaponActor::Fire(ACharacter* Character, const FVector& Start, const FVector& Direction) const
{
    const float ShootRayLength = 10000.0f;
    const FVector End = Start + Direction * ShootRayLength;

    // GetWorld()->DebugDrawTraceTag = FName("Bullet");
    FCollisionQueryParams QueryParams(FName("Bullet"), true, this);
    QueryParams.bReturnPhysicalMaterial = true;
    QueryParams.bTraceComplex = true;
    FHitResult Hit(ForceInit);
    GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams);
    if (Hit.GetActor() == nullptr)
    {
        return;
    }

    if (HitParticle)
    {
        FTransform ParticleTrans(Hit.Location);
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, ParticleTrans);
    }

    if (Hit.PhysMaterial.IsValid())
    {
        switch (Hit.PhysMaterial->SurfaceType)
        {
            case SurfaceType1: // Head
                // GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, "Hit Head");
                if (SoundHelper)
                {
                    SoundHelper->PlaySound("Head");
                }
                break;
            case SurfaceType2: // Body
                // GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, "Hit Body");
                if (SoundHelper)
                {
                    SoundHelper->PlaySound("Body");
                }
                break;
            case SurfaceType3: // Hand
                // GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, "Hit Hand");
                if (SoundHelper)
                {
                    SoundHelper->PlaySound("Hand");
                }
                break;
            case SurfaceType4: // Arm
                // GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, "Hit Arm");
                if (SoundHelper)
                {
                    SoundHelper->PlaySound("Arm");
                }
                break;
            case SurfaceType5: // Foot
                // GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, "Hit Foot");
                if (SoundHelper)
                {
                    SoundHelper->PlaySound("Foot");
                }
                break;
            case SurfaceType6: // Thigh
                // GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, "Hit Thigh");
                if (SoundHelper)
                {
                    SoundHelper->PlaySound("Thigh");
                }
                break;
            case SurfaceType7: // Target
                HitTarget(Character, Hit);
                break;
            default:
                ;
        }
    }
}

void AGunWeaponActor::HitTarget(ACharacter* Character, const FHitResult& Hit) const
{
    AShootingCharacter* ShootingCharacter = Cast<AShootingCharacter>(Character);
    if (ShootingCharacter == nullptr)
    {
        return;
    }
        
    const FVector TargetLocation = Hit.GetActor()->GetActorLocation();
    const float Diff = (TargetLocation - Hit.Location).Size();
    const float Radius = 80.0f;
    const float RadiusInner = 30.0f;
    uint32 Score;
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
    
    AShootingPlayerState* ShootingPlayerState = ShootingCharacter->GetPlayerState<AShootingPlayerState>();
    ShootingPlayerState->IncreaseScore(Score);
}

void AGunWeaponActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    SoundHelper->Destroy();
    Super::EndPlay(EndPlayReason);
}


