// GregrTown - game

#include "Game/GregrPlayerController.h"

#include "EnhancedInputComponent.h"
#include "Game/GregrGameState.h"
#include "Game/GregrGameUserSettings.h"
#include "Kismet/GameplayStatics.h"
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

void AGregrPlayerController::OpenSettings()
{
	if (AGregrHUD* HUD = GetGregrHUD())
	{
		HUD->ShowSettings(true);
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
		SetPause(false);
		SetMenuInputMode(false);
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
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
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
