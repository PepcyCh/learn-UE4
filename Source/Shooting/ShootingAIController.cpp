// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingAIController.h"

#include "ShootingCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"

AShootingAIController::AShootingAIController()
{
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AShootingAIController::OnTargetPerceptionUpdated);
	SetPerceptionComponent(*PerceptionComp);
}

void AShootingAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	RunBehaviorTree(BehaviorTree);
}

void AShootingAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return;
	}

	if (Actor && Actor->ActorHasTag("Player") && Stimulus.WasSuccessfullySensed())
	{
		BlackboardComp->SetValueAsBool(SightOfEnemy_Key, true);
		BlackboardComp->SetValueAsObject(Enemy_Key, Actor);
		bCanViewEnemy = true;
	}
	else
	{
		BlackboardComp->SetValueAsBool(SightOfEnemy_Key, false);
		BlackboardComp->SetValueAsObject(Enemy_Key, nullptr);
		bCanViewEnemy = false;
	}
}

void AShootingAIController::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);
	
	AShootingCharacter* Char = Cast<AShootingCharacter>(GetPawn());
	if (Char)
	{
		UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool(HasValidWeapon_Key, Char->HasValidWeapon());
			BlackboardComp->SetValueAsBool(NeedToEscape_Key, Char->GetHealth() < 50);
			AActor* Enemy = Cast<AActor>(BlackboardComp->GetValueAsObject(Enemy_Key));
			if (bCanViewEnemy && Enemy)
			{
				const FVector SelfLocation = Char->GetActorLocation();
				const FVector EnemyLocation = Enemy->GetActorLocation();
				const float Dist = (SelfLocation - EnemyLocation).Size();
				BlackboardComp->SetValueAsBool(NeedToChase_Key, Dist > ChaseRadius);
			}
			else
			{
				BlackboardComp->SetValueAsBool(NeedToChase_Key, false);
			}
		}
	}
}

