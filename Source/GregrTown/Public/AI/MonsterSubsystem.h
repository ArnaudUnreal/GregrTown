// GregrTown - AI

#pragma once

#include "CoreMinimal.h"
#include "AI/GregrMonster.h"
#include "Subsystems/WorldSubsystem.h"
#include "MonsterSubsystem.generated.h"

class UNavigationSystemV1;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterKilled, EMonsterKind, Kind, int32, KillCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterWaveStarted, int32, Wave, int32, MonsterCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterWaveCleared, int32, Wave);

/** Which monsters and where they appear (set on the game mode). How many is in UMonsterWaveSettings. */
USTRUCT(BlueprintType)
struct GREGRTOWN_API FMonsterSpawnSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monsters")
	TSubclassOf<AGregrMonster> SmallMonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monsters")
	TSubclassOf<AGregrMonster> BigMonsterClass;

	/** No monster appears closer than this to the player (2D). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monsters", meta = (ClampMin = "0"))
	float MinDistanceFromPlayer = 1000.f;

	/** Share of the monsters appearing in a house (houses are marked with Target Points). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monsters|Houses", meta = (ClampMin = "0", ClampMax = "1"))
	float HouseSpawnChance = 0.2f;

	/** Around the house Target Point, on the navmesh reachable from it (so inside the house). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monsters|Houses", meta = (ClampMin = "0"))
	float HouseSpawnRadius = 250.f;

	/** Surface of the water: nothing spawns below. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monsters")
	float WaterLevel = 750.f;

	/** Minimum distance between two monsters of a wave. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monsters", meta = (ClampMin = "0"))
	float MinSpacing = 150.f;
};

/**
 * Keeps track of the monsters of the world and sends them in waves (UMonsterWaveSettings):
 * a wave arrives over a few frames, and the next one is announced by a countdown once the last
 * monster of the current wave is dead. Monsters are not pooled: a wave is at most ~125 actors,
 * spawned a few per frame, and the dead ones vanish after their death animation.
 */
UCLASS()
class GREGRTOWN_API UMonsterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Starts the wave cycle with the first wave after UMonsterWaveSettings::FirstWaveDelay. */
	UFUNCTION(BlueprintCallable, Category = "Monsters|Waves")
	void StartWaves(const FMonsterSpawnSettings& Settings);

	/** No more waves (end of the challenge). Living monsters stay. */
	UFUNCTION(BlueprintCallable, Category = "Monsters|Waves")
	void StopWaves();

	/** Debug / tests: starts the next wave now. */
	UFUNCTION(BlueprintCallable, Category = "Monsters|Waves")
	void StartNextWave();

	/** Current (or last) wave, 0 before the first one. */
	UFUNCTION(BlueprintPure, Category = "Monsters|Waves")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category = "Monsters|Waves")
	bool AreWavesRunning() const { return bWavesRunning; }

	/** True while the countdown to the next wave runs. */
	UFUNCTION(BlueprintPure, Category = "Monsters|Waves")
	bool IsWaitingForNextWave() const;

	/** Seconds before the next wave (0 when no countdown). */
	UFUNCTION(BlueprintPure, Category = "Monsters|Waves")
	float GetTimeUntilNextWave() const;

	/** True until every monster of the current wave has been spawned. */
	UFUNCTION(BlueprintPure, Category = "Monsters|Waves")
	bool IsSpawningWave() const { return SpawnQueue.Num() > 0; }

	/** Spawns one monster standing on GroundLocation (null if the place is blocked). */
	UFUNCTION(BlueprintCallable, Category = "Monsters")
	AGregrMonster* SpawnMonster(TSubclassOf<AGregrMonster> MonsterClass, FVector GroundLocation, float Yaw = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Monsters")
	void DestroyAllMonsters();

	UFUNCTION(BlueprintPure, Category = "Monsters")
	TArray<AGregrMonster*> GetMonsters() const;

	/** Living monsters of a kind. */
	UFUNCTION(BlueprintPure, Category = "Monsters")
	int32 GetMonsterCount(EMonsterKind Kind) const;

	/** Living monsters of all kinds. */
	UFUNCTION(BlueprintPure, Category = "Monsters")
	int32 GetAliveMonsterCount() const;

	UFUNCTION(BlueprintPure, Category = "Monsters")
	int32 GetKillCount(EMonsterKind Kind) const;

	/** Monsters of the current wave placed in houses. */
	UFUNCTION(BlueprintPure, Category = "Monsters")
	int32 GetHouseSpawnCount() const { return HouseSpawnCount; }

	void RegisterMonster(AGregrMonster* Monster);
	void UnregisterMonster(AGregrMonster* Monster);
	void NotifyMonsterKilled(AGregrMonster* Monster);

	UPROPERTY(BlueprintAssignable, Category = "Monsters")
	FOnMonsterKilled OnMonsterKilled;

	UPROPERTY(BlueprintAssignable, Category = "Monsters|Waves")
	FOnMonsterWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Monsters|Waves")
	FOnMonsterWaveCleared OnWaveCleared;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** Spawns the next monsters of SpawnQueue (a few per frame). */
	void SpawnQueuedMonsters();

	/** Starts the countdown to the next wave when the current one is over. */
	void CheckWaveCleared();

	/** A free spot for a wave monster, in a house or anywhere reachable around the player. */
	bool FindSpawnLocation(const UNavigationSystemV1& Navigation, const FVector& PlayerLocation, FVector& OutLocation, bool& bOutInHouse) const;

	TArray<TWeakObjectPtr<AGregrMonster>> Monsters;
	TMap<EMonsterKind, int32> KillCounts;

	FMonsterSpawnSettings SpawnSettings;
	TArray<TSubclassOf<AGregrMonster>> SpawnQueue;
	TArray<FVector> WaveSpawnLocations;
	int32 CurrentWave = 0;
	int32 HouseSpawnCount = 0;
	bool bWavesRunning = false;
	FTimerHandle NextWaveTimer;
	FTimerHandle SpawnTimer;
};
