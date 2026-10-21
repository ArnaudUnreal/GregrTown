// GregrTown - game

#include "Game/GregrPlayerController.h"

#include "EnhancedInputComponent.h"
#include "Game/GregrGameMode.h"
#include "Game/GregrGameState.h"
#include "Game/GregrGameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/GregrHUD.h"

void AGregrPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController())
	{
		return;
	}

	SetMenuInputMode(false);

	if (const UGregrGameUserSettings* Settings = UGregrGameUserSettings::GetGregrUserSettings())
	{
		Settings->ApplyAudio(this);
	}

	if (AGregrGameState* GameState = GetWorld()->GetGameState<AGregrGameState>())
	{
		GameState->OnChallengeFinished.AddUniqueDynamic(this, &AGregrPlayerController::HandleChallengeFinished);
	}
}

void AGregrPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent); Input && SettingsAction)
	{
		// The action asset must have "Trigger when Paused" enabled to close the menu while paused.
		Input->BindAction(SettingsAction, ETriggerEvent::Started, this, &AGregrPlayerController::ToggleSettings);
	}
}

AGregrHUD* AGregrPlayerController::GetGregrHUD() const
{
	return Cast<AGregrHUD>(GetHUD());
}

bool AGregrPlayerController::IsChallengeFinished() const
{
	const AGregrGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AGregrGameState>() : nullptr;
	return GameState && GameState->IsChallengeFinished();
}

bool AGregrPlayerController::IsChallengeStarted() const
{
	const AGregrGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AGregrGameState>() : nullptr;
	return GameState && (GameState->IsChallengeRunning() || GameState->IsChallengeFinished());
}

void AGregrPlayerController::OpenStartMenu()
{
	if (AGregrHUD* HUD = GetGregrHUD())
	{
		HUD->ShowSettings(true, true);
	}
	SetPause(true);
	SetMenuInputMode(true);
}

void AGregrPlayerController::OpenSettings()
{
	if (AGregrHUD* HUD = GetGregrHUD())
	{
		HUD->ShowSettings(true, !IsChallengeStarted());
	}
	if (!IsChallengeFinished())
	{
		SetPause(true);
	}
	SetMenuInputMode(true);
}

void AGregrPlayerController::CloseSettings()
{
	if (UGregrGameUserSettings* Settings = UGregrGameUserSettings::GetGregrUserSettings())
	{
		Settings->SaveSettings();
	}
	if (AGregrHUD* HUD = GetGregrHUD())
	{
		HUD->ShowSettings(false);
	}

	// Back to the end screen if the challenge is over, back to the game otherwise.
	if (!IsChallengeFinished())
	{
		const bool bStart = !IsChallengeStarted();
		SetPause(false);
		SetMenuInputMode(false);
		if (AGregrGameMode* GameMode = bStart ? GetWorld()->GetAuthGameMode<AGregrGameMode>() : nullptr)
		{
			GameMode->StartChallenge();
		}
	}
}

void AGregrPlayerController::ToggleSettings()
{
	const AGregrHUD* HUD = GetGregrHUD();
	if (HUD && HUD->IsSettingsVisible())
	{
		CloseSettings();
	}
	else
	{
		OpenSettings();
	}
}

void AGregrPlayerController::RestartChallenge()
{
	if (UGregrGameUserSettings* Settings = UGregrGameUserSettings::GetGregrUserSettings())
	{
		Settings->SaveSettings();
	}
	SetPause(false);
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)), true, AGregrGameMode::PlayNowOption);
}

void AGregrPlayerController::QuitToDesktop()
{
	if (UGregrGameUserSettings* Settings = UGregrGameUserSettings::GetGregrUserSettings())
	{
		Settings->SaveSettings();
	}
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void AGregrPlayerController::HandleChallengeFinished(int32 FinalScore)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->DisableInput(this);
	}
	if (AGregrHUD* HUD = GetGregrHUD())
	{
		HUD->ShowEndScreen(FinalScore);
	}
	SetMenuInputMode(true);
}

void AGregrPlayerController::SetMenuInputMode(bool bMenu)
{
	if (bMenu)
	{
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		SetInputMode(Mode);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}
	SetShowMouseCursor(bMenu);
}
