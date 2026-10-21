// GregrTown - AI

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MonsterWaveSettings.generated.h"

/**
 * Project Settings > Game > GregrTown Monster Waves (saved in DefaultGame.ini).
 * Waves grow from FirstWaveSmallMonsters by SmallMonstersAddedPerWave up to MaxWaveSmallMonsters,
 * then every following wave has MaxWaveSmallMonsters. The next wave comes DelayBetweenWaves after
 * the last monster of the current one dies.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "GregrTown Monster Waves"))
class GREGRTOWN_API UMonsterWaveSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMonsterWaveSettings();

	/** Small monsters of the first wave. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Waves", meta = (ClampMin = "1"))
	int32 FirstWaveSmallMonsters = 10;

	/** Small monsters of the hardest wave (and of every wave after it). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Waves", meta = (ClampMin = "1"))
	int32 MaxWaveSmallMonsters = 100;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Waves", meta = (ClampMin = "0"))
	int32 SmallMonstersAddedPerWave = 10;

	/** One big monster for this many small ones in each wave. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Waves", meta = (ClampMin = "1"))
	int32 SmallPerBig = 4;

	/** Countdown between the death of the last monster of a wave and the next wave (seconds). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Waves", meta = (ClampMin = "0", Units = "Seconds"))
	float DelayBetweenWaves = 20.f;

	/** Countdown before the first wave, once the challenge starts (seconds). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Waves", meta = (ClampMin = "0", Units = "Seconds"))
	float FirstWaveDelay = 5.f;

	/** Monsters spawned per frame while a wave arrives (spreads the cost of big waves). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Waves", meta = (ClampMin = "1"))
	int32 SpawnsPerFrame = 10;

	/** Small and big monsters of a wave (1 = first wave). */
	UFUNCTION(BlueprintPure, Category = "Waves")
	void GetWaveSize(int32 Wave, int32& OutSmall, int32& OutBig) const;

	static const UMonsterWaveSettings* Get() { return GetDefault<UMonsterWaveSettings>(); }
};
