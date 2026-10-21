// GregrTown - AI

#include "AI/MonsterSubsystem.h"

#include "AI/MonsterWaveSettings.h"
#include "Components/CapsuleComponent.h"
#include "Engine/TargetPoint.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogGregrMonsters, Log, All);

void UMonsterSubsystem::StartWaves(const FMonsterSpawnSettings& Settings)
{
	SpawnSettings = Settings;
	CurrentWave = 0;
	bWavesRunning = true;
	GetWorld()->GetTimerManager().SetTimer(NextWaveTimer, this, &UMonsterSubsystem::StartNextWave,
		FMath::Max(UMonsterWaveSettings::Get()->FirstWaveDelay, 0.01f), false);
}

void UMonsterSubsystem::StopWaves()
{
	bWavesRunning = false;
	SpawnQueue.Reset();
	GetWorld()->GetTimerManager().ClearTimer(NextWaveTimer);
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
}

void UMonsterSubsystem::StartNextWave()
{
	if (!bWavesRunning)
	{
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(NextWaveTimer);

	++CurrentWave;
	int32 SmallCount = 0;
	int32 BigCount = 0;
	UMonsterWaveSettings::Get()->GetWaveSize(CurrentWave, SmallCount, BigCount);

	// Random order, so big monsters are spread among the small ones (and in houses).
	SpawnQueue.Reset();
	if (SpawnSettings.SmallMonsterClass)
	{
		SpawnQueue.Init(SpawnSettings.SmallMonsterClass, SmallCount);
	}
	for (int32 Index = 0; SpawnSettings.BigMonsterClass && Index < BigCount; ++Index)
	{
		SpawnQueue.Add(SpawnSettings.BigMonsterClass);
	}
	for (int32 Index = SpawnQueue.Num() - 1; Index > 0; --Index)
	{
		SpawnQueue.Swap(Index, FMath::RandRange(0, Index));
	}

	WaveSpawnLocations.Reset();
	HouseSpawnCount = 0;
	UE_LOG(LogGregrMonsters, Log, TEXT("Wave %d: %d small, %d big"), CurrentWave, SmallCount, BigCount);
	OnWaveStarted.Broadcast(CurrentWave, SpawnQueue.Num());

	SpawnQueuedMonsters();
	if (SpawnQueue.Num() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &UMonsterSubsystem::SpawnQueuedMonsters, 0.01f, true);
	}
}

void UMonsterSubsystem::SpawnQueuedMonsters()
{
	UWorld* World = GetWorld();
	const UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	const APlayerController* PC = World->GetFirstPlayerController();
	const APawn* Player = PC ? PC->GetPawn() : nullptr;
	if (!Navigation || !Player)
	{
		UE_LOG(LogGregrMonsters, Warning, TEXT("Wave %d: no navigation or no player, %d monsters skipped"), CurrentWave, SpawnQueue.Num());
		SpawnQueue.Reset();
	}

	for (int32 Count = 0; Count < UMonsterWaveSettings::Get()->SpawnsPerFrame && SpawnQueue.Num() > 0; ++Count)
	{
		const TSubclassOf<AGregrMonster> MonsterClass = SpawnQueue.Pop();
		// A few tries to find a free spot; a monster that finds none is dropped from the wave.
		for (int32 Attempt = 0; Attempt < 20; ++Attempt)
		{
			FVector Location;
			bool bInHouse = false;
			if (FindSpawnLocation(*Navigation, Player->GetActorLocation(), Location, bInHouse) && SpawnMonster(MonsterClass, Location, FMath::FRandRange(0.f, 360.f)))
			{
				WaveSpawnLocations.Add(Location);
				HouseSpawnCount += bInHouse ? 1 : 0;
				break;
			}
		}
	}

	if (SpawnQueue.Num() == 0)
	{
		World->GetTimerManager().ClearTimer(SpawnTimer);
		CheckWaveCleared();
	}
}

bool UMonsterSubsystem::FindSpawnLocation(const UNavigationSystemV1& Navigation, const FVector& PlayerLocation, FVector& OutLocation, bool& bOutInHouse) const
{
	const FVector ProjectExtent(200.f, 200.f, 400.f);
	FVector Origin = PlayerLocation;
	float Radius = 100000.f;
	bool bInHouse = false;

	if (FMath::FRand() < SpawnSettings.HouseSpawnChance)
	{
		// A random house (Target Point) far enough from the player.
		TArray<const ATargetPoint*> Houses;
		for (TActorIterator<ATargetPoint> It(GetWorld()); It; ++It)
		{
			if (FVector::Dist2D(It->GetActorLocation(), PlayerLocation) >= SpawnSettings.MinDistanceFromPlayer)
			{
				Houses.Add(*It);
			}
		}
		if (Houses.Num() > 0)
		{
			Origin = Houses[FMath::RandRange(0, Houses.Num() - 1)]->GetActorLocation();
			Radius = SpawnSettings.HouseSpawnRadius;
			bInHouse = true;
		}
	}

	// Anywhere else: reachable from the player (no roof, no closed room).
	FNavLocation OriginOnNav;
	FNavLocation Location;
	if (!Navigation.ProjectPointToNavigation(Origin, OriginOnNav, ProjectExtent)
		|| !Navigation.GetRandomReachablePointInRadius(OriginOnNav.Location, Radius, Location))
	{
		return false;
	}

	if (Location.Location.Z < SpawnSettings.WaterLevel || FVector::Dist2D(Location.Location, PlayerLocation) < SpawnSettings.MinDistanceFromPlayer)
	{
		return false;
	}
	for (const FVector& Other : WaveSpawnLocations)
	{
		if (FVector::DistSquared(Location.Location, Other) < FMath::Square(SpawnSettings.MinSpacing))
		{
			return false;
		}
	}

	OutLocation = Location.Location;
	bOutInHouse = bInHouse;
	return true;
}

void UMonsterSubsystem::CheckWaveCleared()
{
	if (!bWavesRunning || GetWorld()->bIsTearingDown || CurrentWave == 0 || IsSpawningWave() || GetAliveMonsterCount() > 0 || IsWaitingForNextWave())
	{
		return;
	}

	OnWaveCleared.Broadcast(CurrentWave);
	GetWorld()->GetTimerManager().SetTimer(NextWaveTimer, this, &UMonsterSubsystem::StartNextWave,
		FMath::Max(UMonsterWaveSettings::Get()->DelayBetweenWaves, 0.01f), false);
}

bool UMonsterSubsystem::IsWaitingForNextWave() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(NextWaveTimer);
}

float UMonsterSubsystem::GetTimeUntilNextWave() const
{
	return IsWaitingForNextWave() ? GetWorld()->GetTimerManager().GetTimerRemaining(NextWaveTimer) : 0.f;
}

AGregrMonster* UMonsterSubsystem::SpawnMonster(TSubclassOf<AGregrMonster> MonsterClass, FVector GroundLocation, float Yaw)
{
	if (!MonsterClass)
	{
		return nullptr;
	}

	const AGregrMonster* Defaults = MonsterClass->GetDefaultObject<AGregrMonster>();
	const float HalfHeight = Defaults->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	return GetWorld()->SpawnActor<AGregrMonster>(MonsterClass, GroundLocation + FVector(0.f, 0.f, HalfHeight + 5.f), FRotator(0.f, Yaw, 0.f), Params);
}

void UMonsterSubsystem::DestroyAllMonsters()
{
	for (AGregrMonster* Monster : GetMonsters())
	{
		Monster->Destroy();
	}
	Monsters.Reset();
}

TArray<AGregrMonster*> UMonsterSubsystem::GetMonsters() const
{
	TArray<AGregrMonster*> Result;
	for (const TWeakObjectPtr<AGregrMonster>& Monster : Monsters)
	{
		if (Monster.IsValid())
		{
			Result.Add(Monster.Get());
		}
	}
	return Result;
}

int32 UMonsterSubsystem::GetMonsterCount(EMonsterKind Kind) const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AGregrMonster>& Monster : Monsters)
	{
		Count += Monster.IsValid() && Monster->Kind == Kind && !Monster->IsDead() ? 1 : 0;
	}
	return Count;
}

int32 UMonsterSubsystem::GetAliveMonsterCount() const
{
	return GetMonsterCount(EMonsterKind::Small) + GetMonsterCount(EMonsterKind::Big);
}

int32 UMonsterSubsystem::GetKillCount(EMonsterKind Kind) const
{
	const int32* Count = KillCounts.Find(Kind);
	return Count ? *Count : 0;
}

void UMonsterSubsystem::RegisterMonster(AGregrMonster* Monster)
{
	Monsters.AddUnique(Monster);
}

void UMonsterSubsystem::UnregisterMonster(AGregrMonster* Monster)
{
	Monsters.Remove(Monster);
	// A monster can also leave without dying (fell out of the world).
	CheckWaveCleared();
}

void UMonsterSubsystem::NotifyMonsterKilled(AGregrMonster* Monster)
{
	int32& Count = KillCounts.FindOrAdd(Monster->Kind);
	++Count;
	OnMonsterKilled.Broadcast(Monster->Kind, Count);
	CheckWaveCleared();
}

bool UMonsterSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
