// GregrTown - UI

#include "UI/GregrHUD.h"

#include "Blueprint/UserWidget.h"
#include "Game/GregrGameState.h"
#include "Game/GregrPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Scoring/ScoreSubsystem.h"
#include "UI/GregrEndScreenWidget.h"
#include "UI/GregrHUDWidget.h"
#include "UI/GregrSettingsWidget.h"

AGregrHUD::AGregrHUD()
{
	HUDWidgetClass = UGregrHUDWidget::StaticClass();
	SettingsWidgetClass = UGregrSettingsWidget::StaticClass();
	EndScreenWidgetClass = UGregrEndScreenWidget::StaticClass();
}

void AGregrHUD::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = GetOwningPlayerController(); PC && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UGregrHUDWidget>(PC, HUDWidgetClass);
		HUDWidgetVisibility = HUDWidget->GetVisibility();
		HUDWidget->AddToViewport(0);
	}

	if (UScoreSubsystem* Score = UWorld::GetSubsystem<UScoreSubsystem>(GetWorld()))
	{
		Score->OnScoreChanged.AddUniqueDynamic(this, &AGregrHUD::HandleScoreChanged);
	}
	if (AGregrGameState* GameState = GetWorld()->GetGameState<AGregrGameState>())
	{
		GameState->OnChallengeTimeChanged.AddUniqueDynamic(this, &AGregrHUD::HandleTimeChanged);
		GameState->OnChallengeFinished.AddUniqueDynamic(this, &AGregrHUD::HandleChallengeFinished);
	}
}

void AGregrHUD::HandleScoreChanged(int32 NewScore, int32 Delta, AActor* Source, AActor* ScoreInstigator, FVector Location)
{
	if (ScoreEffect && Delta > 0 && !Location.IsZero())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ScoreEffect, Location);
	}

	const double Now = GetWorld()->GetTimeSeconds();
	if (ScoreSound && Delta > 0 && Now - LastScoreSoundTime > 0.06)
	{
		LastScoreSoundTime = Now;
		const float Pitch = FMath::GetMappedRangeValueClamped(FVector2f(5.f, 100.f), FVector2f(0.9f, 1.5f), static_cast<float>(Delta));
		UGameplayStatics::PlaySound2D(this, ScoreSound, 0.8f, Pitch);
	}
}

void AGregrHUD::HandleTimeChanged(int32 SecondsLeft)
{
	if (CountdownTickSound && SecondsLeft > 0 && SecondsLeft <= CountdownTickSeconds)
	{
		UGameplayStatics::PlaySound2D(this, CountdownTickSound, 0.7f, SecondsLeft <= 3 ? 1.3f : 1.f);
	}
}

void AGregrHUD::HandleChallengeFinished(int32 FinalScore)
{
	if (ChallengeEndSound)
	{
		UGameplayStatics::PlaySound2D(this, ChallengeEndSound);
	}
}

void AGregrHUD::ShowSettings(bool bShow, bool bStartMenu)
{
	AGregrPlayerController* PC = Cast<AGregrPlayerController>(GetOwningPlayerController());
	if (bShow && !SettingsWidget && PC && SettingsWidgetClass)
	{
		SettingsWidget = CreateWidget<UGregrSettingsWidget>(PC, SettingsWidgetClass);
		SettingsWidget->OnResumeRequested.AddDynamic(PC, &AGregrPlayerController::CloseSettings);
		SettingsWidget->OnRestartRequested.AddDynamic(PC, &AGregrPlayerController::RestartChallenge);
		SettingsWidget->OnQuitRequested.AddDynamic(PC, &AGregrPlayerController::QuitToDesktop);
	}

	// Score and countdown mean nothing before the challenge starts.
	if (HUDWidget)
	{
		HUDWidget->SetVisibility(bShow && bStartMenu ? ESlateVisibility::Collapsed : HUDWidgetVisibility);
	}
	if (!SettingsWidget)
	{
		return;
	}

	if (bShow)
	{
		SettingsWidget->SetStartMenu(bStartMenu);
		SettingsWidget->RefreshFromSettings();
		if (!SettingsWidget->IsInViewport())
		{
			SettingsWidget->AddToViewport(20);
		}
	}
	else
	{
		SettingsWidget->RemoveFromParent();
	}
}

void AGregrHUD::ShowEndScreen(int32 FinalScore)
{
	AGregrPlayerController* PC = Cast<AGregrPlayerController>(GetOwningPlayerController());
	if (!EndScreenWidget && PC && EndScreenWidgetClass)
	{
		EndScreenWidget = CreateWidget<UGregrEndScreenWidget>(PC, EndScreenWidgetClass);
		EndScreenWidget->OnRestartRequested.AddDynamic(PC, &AGregrPlayerController::RestartChallenge);
		EndScreenWidget->OnSettingsRequested.AddDynamic(PC, &AGregrPlayerController::OpenSettings);
	}
	if (EndScreenWidget)
	{
		EndScreenWidget->SetFinalScore(FinalScore);
		const AGregrGameState* GameState = GetWorld()->GetGameState<AGregrGameState>();
		EndScreenWidget->SetNewRecord(GameState && GameState->IsNewRecord());
		if (!EndScreenWidget->IsInViewport())
		{
			EndScreenWidget->AddToViewport(10);
		}
	}
}

bool AGregrHUD::IsSettingsVisible() const
{
	return SettingsWidget && SettingsWidget->IsInViewport();
}

bool AGregrHUD::IsEndScreenVisible() const
{
	return EndScreenWidget && EndScreenWidget->IsInViewport();
}
