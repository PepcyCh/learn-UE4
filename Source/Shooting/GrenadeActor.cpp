// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeActor.h"

#include "ExplosionActor.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AGrenadeActor::AGrenadeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	RootComponent = GrenadeMesh;

	GrenadeMesh->OnComponentHit.AddDynamic(this, &AGrenadeActor::OnHit);
}

// Called when the game starts or when spawned
void AGrenadeActor::BeginPlay()
{
	Super::BeginPlay();

	CreatedTime = FDateTime::Now();
	SetLifeSpan(10.0f);
}

// Called every frame
void AGrenadeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FDateTime CurrentTime = FDateTime::Now();
	if (CurrentTime - CreatedTime < FTimespan::FromSeconds(0.5f))
	{
		return;
	}

	const float Speed = GetVelocity().Size();
	const static float ExplosionSpeed = 120.0f;
	if (Speed <= ExplosionSpeed)
	{
		EmitExplosion();
	}
}

void AGrenadeActor::EmitExplosion()
{
	if (Explosion)
	{
		GetWorld()->SpawnActor<AExplosionActor>(Explosion, GetActorLocation(), FRotator());
	}
	GetWorld()->DestroyActor(this);
}


void AGrenadeActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		if (Hit.PhysMaterial.IsValid())
		{
			switch (Hit.PhysMaterial->SurfaceType)
			{
				case SurfaceType1: // Head
				case SurfaceType2: // Body
				case SurfaceType3: // Hand
				case SurfaceType4: // Arm
				case SurfaceType5: // Foot
				case SurfaceType6: // Thigh
					EmitExplosion();
					break;
				default:
					const float Friction = Hit.PhysMaterial->Friction;
					GrenadeMesh->AddForce(-Friction * GetVelocity());
			}			
		}
	}
}

