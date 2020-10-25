// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingPlayerState.h"

#include "Net/UnrealNetwork.h"

AShootingPlayerState::AShootingPlayerState()
{
    bReplicates = true;
}

void AShootingPlayerState::IncreaseScore(uint32 delta)
{
    if (!GetWorld()->IsServer())
    {
        IncreaseScoreServer(delta);
    }
    else
    {
        ShootingScore += delta;
    }
}
void AShootingPlayerState::IncreaseScoreServer_Implementation(uint32 delta)
{
    ShootingScore += delta;
}
bool AShootingPlayerState::IncreaseScoreServer_Validate(uint32 delta)
{
    return delta <= 10;
}

uint32 AShootingPlayerState::GetShootingScore() const
{
    return ShootingScore;
}

void AShootingPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AShootingPlayerState, ShootingScore);
    DOREPLIFETIME(AShootingPlayerState, Id);
}