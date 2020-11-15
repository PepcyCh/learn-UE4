// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingHUD.h"

#include "ShootingPlayerState.h"
#include "ShootingGameState.h"
#include "Engine/Canvas.h"

#include "ShootingGameInstance.h"
#include "ShootingCharacter.h"
#include "UObject/ConstructorHelpers.h"

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
        FString PlayerName = Cast<UShootingGameInstance>(GetGameInstance())->GetPlayerName();
        FText PlayerText = FText::Format(FTextFormat::FromString("Current Player - {0}"), FText::FromString(PlayerName));
        DrawText(PlayerText.ToString(), FColor::Orange, 10.0f, ScoreTextY, Font, 3);
        ScoreTextY += 40.0f;
    }
    for (AShootingPlayerState* PlayerState : ShootingPlayerArray)
    {
        FString PlayerName = PlayerState->GetPlayerName();
        uint32 CurrentScore = PlayerState->GetShootingScore();
        FText ScoreText = FText::Format(FTextFormat::FromString("{0} - Score: {1}"), FText::FromString(PlayerName), CurrentScore);
        DrawText(ScoreText.ToString(), FColor::Orange, 10.0f, ScoreTextY, Font, 3);
        ScoreTextY += 40.0f;
    }
    {
        int32 Time = Cast<AShootingCharacter>(GetOwningPlayerController()->GetPawn())->GetTimeRemaining();
        if (Time < 0)
        {
            DrawText("00:00", FColor::Red, Canvas->ClipX - 150.0f, 10.0f, Font, 3);
        }
        else
        {
            int32 Minute = Time / 60;
            int32 Second = Time % 60;
            FNumberFormattingOptions Options;
            Options.MinimumIntegralDigits = 2;
            FText TimeText = FText::Format(FTextFormat::FromString("{0}:{1}"), FText::AsNumber(Minute, &Options), FText::AsNumber(Second, &Options));
            DrawText(TimeText.ToString(), FColor::Blue, Canvas->ClipX - 150.0f, 10.0f, Font, 3);
        }
    }
}
