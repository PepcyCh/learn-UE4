// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"

#include "ShootingAIController.generated.h"

UCLASS()
class SHOOTING_API AShootingAIController : public AAIController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAIPerceptionComponent* PerceptionComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SightOfEnemy_Key = "SightOfEnemy";
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Enemy_Key = "Enemy";
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HasValidWeapon_Key = "HasValidWeapon";
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NeedToEscape_Key = "NeedToEscape";
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NeedToChase_Key = "NeedToChase";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChaseRadius = 600.0f;

	UPROPERTY(VisibleAnywhere)
	bool bCanViewEnemy = false;

public:
	AShootingAIController();
	
	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaSecond) override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

};
