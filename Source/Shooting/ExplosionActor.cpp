// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosionActor.h"

#include "Particles/ParticleSystemComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AExplosionActor::AExplosionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	Particle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ExplosionParticle"));
	Particle->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void AExplosionActor::BeginPlay()
{
	Super::BeginPlay();

	Particle->Activate();

	TArray<FHitResult> OutHits;

	const FVector Location = GetActorLocation();
	const FVector Start = Location;
	const FVector End = Location;

	const FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(ExplosionRadius);
	const bool bIsHit = GetWorld()->SweepMultiByChannel(OutHits, Start, End, FQuat::Identity,
		ECC_PhysicsBody, CollisionSphere);

	if (bIsHit)
	{
		for (const FHitResult& Hit : OutHits)
		{
			UStaticMeshComponent* MeshCompo = Cast<UStaticMeshComponent>(Hit.GetActor()->GetRootComponent());
			if (MeshCompo)
			{
				MeshCompo->AddRadialImpulse(Location, ExplosionRadius, ExplosionImpulse, RIF_Constant);
				SetLifeSpan(2.0f);
			}
		}
	}
}

// Called every frame
void AExplosionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

