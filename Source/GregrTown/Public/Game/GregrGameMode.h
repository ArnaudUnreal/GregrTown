// GregrTown - game

#pragma once

#include "CoreMinimal.h"
#include "AI/MonsterSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "GregrGameMode.generated.h"

/**
 * Base game mode of GregrTown: opens the start menu (settings before playing), then runs the timed
 * challenge and its monster waves. Pawn and monster classes are set in its Blueprint child.
 */
UCLASS(Blueprintable)
class GREGRTOWN_API AGregrGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGregrGameMode();

	virtual void StartPlay() override;

	/** Starts the countdown with the duration of the settings. */
	UFUNCTION(BlueprintCallable, Category = "Challenge")
	void StartChallenge();

	/** Sends the monster waves (done when the challenge starts unless gregr.Monsters is 0). */
	UFUNCTION(BlueprintCallable, Category = "Monsters")
	void StartMonsterWaves();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monsters")
	FMonsterSpawnSettings MonsterSpawn;

	/** Level URL option set by "Replay": the challenge starts right away, without the start menu. */
	static const TCHAR* PlayNowOption;

protected:
	UFUNCTION()
	void HandleChallengeFinished(int32 FinalScore);
};
