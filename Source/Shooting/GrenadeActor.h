// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/DateTime.h"
#include "GrenadeActor.generated.h"

UCLASS()
class SHOOTING_API AGrenadeActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	FDateTime CreatedTime;
	
public:	
	// Sets default values for this actor's properties
	AGrenadeActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	AController* OwnerController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* GrenadeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AExplosionActor> Explosion;

	UFUNCTION(Server, Reliable)
    void EmitExplosion_Server();
	void EmitExplosion_Server_Implementation();

	UFUNCTION(Server, Reliable)
	void HitCharacter_Server(AActor* OtherActor);
	void HitCharacter_Server_Implementation(AActor* OtherActor);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetOwnerController(AController* Controller);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	UStaticMeshComponent* GetGrenadeMeshComponent() const { return GrenadeMesh; }
};
