// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponActor.h"

AWeaponActor::AWeaponActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = StaticMeshComponent;

	StaticMeshComponent->SetGenerateOverlapEvents(true);
	StaticMeshComponent->SetCollisionProfileName("OverlapAll");
}

void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeaponActor::Fire(ACharacter* Character, const FVector& Start, const FVector& Direction) const
{
}
