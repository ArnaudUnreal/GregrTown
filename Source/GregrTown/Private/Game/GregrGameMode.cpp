// GregrTown - game

#include "Game/GregrGameMode.h"

#include "Game/GregrGameState.h"
#include "Game/GregrGameUserSettings.h"
#include "Game/GregrPlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/GregrHUD.h"

const TCHAR* AGregrGameMode::PlayNowOption = TEXT("PlayNow");

static TAutoConsoleVariable<bool> CVarSpawnMonsters(
	TEXT("gregr.Monsters"),
	true,
	TEXT("Send the monster waves when a challenge starts (the tests turn it off and spawn their own)."));

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
		// 0: no time limit.
		Challenge->StartChallenge(!Settings ? 120.f : Settings->IsChallengeUnlimited() ? 0.f : Settings->GetChallengeDuration());
		Challenge->OnChallengeFinished.AddUniqueDynamic(this, &AGregrGameMode::HandleChallengeFinished);
	}
	if (CVarSpawnMonsters.GetValueOnGameThread())
	{
		StartMonsterWaves();
	}
}

void AGregrGameMode::StartMonsterWaves()
{
	if (UMonsterSubsystem* Monsters = UWorld::GetSubsystem<UMonsterSubsystem>(GetWorld()))
	{
		Monsters->StartWaves(MonsterSpawn);
	}
}

void AGregrGameMode::HandleChallengeFinished(int32 FinalScore)
{
	if (UMonsterSubsystem* Monsters = UWorld::GetSubsystem<UMonsterSubsystem>(GetWorld()))
	{
		Monsters->StopWaves();
	}
}
