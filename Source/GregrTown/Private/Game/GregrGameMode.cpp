// GregrTown - game

#include "Game/GregrGameMode.h"

#include "Game/GregrGameState.h"
#include "Game/GregrGameUserSettings.h"
#include "Game/GregrPlayerController.h"
#include "UI/GregrHUD.h"

AGregrGameMode::AGregrGameMode()
{
	PlayerControllerClass = AGregrPlayerController::StaticClass();
	GameStateClass = AGregrGameState::StaticClass();
	HUDClass = AGregrHUD::StaticClass();
}

void AGregrGameMode::StartPlay()
{
	Super::StartPlay();

	if (AGregrGameState* Challenge = GetGameState<AGregrGameState>())
	{
		const UGregrGameUserSettings* Settings = UGregrGameUserSettings::GetGregrUserSettings();
		Challenge->StartChallenge(Settings ? Settings->GetChallengeDuration() : 120.f);
	}
}
