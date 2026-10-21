// GregrTown - AI

#include "AI/MonsterWaveSettings.h"

UMonsterWaveSettings::UMonsterWaveSettings()
{
	CategoryName = TEXT("Game");
}

void UMonsterWaveSettings::GetWaveSize(int32 Wave, int32& OutSmall, int32& OutBig) const
{
	const int32 Max = FMath::Max(MaxWaveSmallMonsters, 1);
	OutSmall = FMath::Clamp(FirstWaveSmallMonsters + (FMath::Max(Wave, 1) - 1) * SmallMonstersAddedPerWave, 1, Max);
	OutBig = FMath::RoundToInt32(static_cast<float>(OutSmall) / FMath::Max(SmallPerBig, 1));
}
