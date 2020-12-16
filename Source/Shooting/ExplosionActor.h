// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExplosionActor.generated.h"

UCLASS()
class SHOOTING_API AExplosionActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneComponent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UParticleSystemComponent* Particle;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplosionRadius = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplosionImpulse = 100000.0f;

	// Sets default values for this actor's properties
	AExplosionActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
