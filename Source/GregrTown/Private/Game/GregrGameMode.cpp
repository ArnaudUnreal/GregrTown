// GregrTown - game

#include "Game/GregrGameMode.h"

#include "Game/GregrGameState.h"
#include "Game/GregrGameUserSettings.h"
#include "Game/GregrPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/GregrHUD.h"

const TCHAR* AGregrGameMode::PlayNowOption = TEXT("PlayNow");

AGregrGameMode::AGregrGameMode()
{
	PlayerControllerClass = AGregrPlayerController::StaticClass();
	GameStateClass = AGregrGameState::StaticClass();
	HUDClass = AGregrHUD::StaticClass();
}

void AGregrGameMode::StartPlay()
{
	Super::StartPlay();

	if (UGameplayStatics::HasOption(OptionsString, PlayNowOption))
	{
		StartChallenge();
	}
	else if (AGregrPlayerController* PC = Cast<AGregrPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->OpenStartMenu();
	}
	else
	{
		StartChallenge();
	}
}

void AGregrGameMode::StartChallenge()
{
	if (AGregrGameState* Challenge = GetGameState<AGregrGameState>())
	{
		const UGregrGameUserSettings* Settings = UGregrGameUserSettings::GetGregrUserSettings();
		Challenge->StartChallenge(Settings ? Settings->GetChallengeDuration() : 120.f);
	}
}
