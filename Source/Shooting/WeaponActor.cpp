// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponActor.h"

#include "Components/StaticMeshComponent.h"

AWeaponActor::AWeaponActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = StaticMeshComponent;

	StaticMeshComponent->SetGenerateOverlapEvents(true);
	StaticMeshComponent->SetCollisionProfileName("OverlapAll");
}

void AWeaponActor::SetOwnerController(AController* Controller)
{
	if (Controller)
	{
		OwnerController = Controller;
	}
}


void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeaponActor::Fire(ACharacter* Character, const FVector& Start, const FVector& Direction)
{
	--BulletCount;
}

FString AWeaponActor::GetWeaponName() const
{
	if (BulletCount <= 0)
	{
		return WeaponName + " (No Bullet !)";
	}
	return WeaponName + " (# of Bullet: " + FString::FromInt(BulletCount) + ")"; 
}

