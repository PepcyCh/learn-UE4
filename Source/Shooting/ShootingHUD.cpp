// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingHUD.h"

#include "ShootingPlayerState.h"
#include "ShootingGameState.h"
#include "Engine/Canvas.h"

AShootingHUD::AShootingHUD()
{
    ConstructorHelpers::FObjectFinder<UFont> FontObject(TEXT("/Game/Fonts/ROBOTO-MEDIUM_Font.ROBOTO-MEDIUM_Font"));
    if (FontObject.Object)
    {
        Font = FontObject.Object;
    }
}

void AShootingHUD::DrawHUD()
{
    Super::DrawHUD();
    FString MapName = GetWorld()->GetMapName().Mid(GetWorld()->StreamingLevelsPrefix.Len());
    if (MapName != "ThirdPersonExampleMap")
    {
        return;
    }
    if (!Font)
    {
        return;
    }

    const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
    DrawText("+", FColor::Red, Center.X, Center.Y, Font, 5);

    TArray<APlayerState*> PlayerArray = GetWorld()->GetGameState()->PlayerArray;
    TArray<AShootingPlayerState*> ShootingPlayerArray;
    for (APlayerState* PlayerState_ : PlayerArray)
    {
        AShootingPlayerState* PlayerState = Cast<AShootingPlayerState>(PlayerState_);
        if (PlayerState)
        {
            ShootingPlayerArray.Add(PlayerState);
        }
    }

    ShootingPlayerArray.Sort([](const AShootingPlayerState& a, const AShootingPlayerState& b)
        {
            if (a.GetShootingScore() == b.GetShootingScore())
            {
                return a.Id < b.Id;
            }
            else
            {
                return a.GetShootingScore() > b.GetShootingScore();
            }
        });
    float ScoreTextY = 10.0f;
    {
        AShootingPlayerState* PlayerState = Cast<AShootingPlayerState>(GetOwningPlayerController()->PlayerState);
        uint32 Id = PlayerState->Id;
        FText PlayerText = FText::Format(FTextFormat::FromString("Current Player - Player {0}"), Id);
        DrawText(PlayerText.ToString(), FColor::Orange, 10, ScoreTextY, Font, 3);
        ScoreTextY += 40.0f;
    }
    for (AShootingPlayerState* PlayerState : ShootingPlayerArray)
    {
        uint32 Id = PlayerState->Id;
        uint32 CurrentScore = PlayerState->GetShootingScore();
        FText ScoreText = FText::Format(FTextFormat::FromString("Player {0} - Score: {1}"), Id, CurrentScore);
        DrawText(ScoreText.ToString(), FColor::Orange, 10, ScoreTextY, Font, 3);
        ScoreTextY += 40.0f;
    }
}