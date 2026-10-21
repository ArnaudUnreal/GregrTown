// GregrTown - scoring

#include "Scoring/RecordsSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Scoring/GregrSaveGame.h"

const FString URecordsSubsystem::SlotName = TEXT("GregrRecords");

void URecordsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SaveGame = Cast<UGregrSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveGame)
	{
		SaveGame = Cast<UGregrSaveGame>(UGameplayStatics::CreateSaveGameObject(UGregrSaveGame::StaticClass()));
	}
}

int32 URecordsSubsystem::GetBestScore(int32 DurationSeconds) const
{
	const int32* Best = SaveGame ? SaveGame->BestScoreByDuration.Find(DurationSeconds) : nullptr;
	return Best ? *Best : 0;
}

bool URecordsSubsystem::SubmitScore(int32 DurationSeconds, int32 Score)
{
	if (!SaveGame || Score <= GetBestScore(DurationSeconds))
	{
		return false;
	}
	SaveGame->BestScoreByDuration.Add(DurationSeconds, Score);
	Save();
	return true;
}

void URecordsSubsystem::SetBestScore(int32 DurationSeconds, int32 Score)
{
	if (!SaveGame)
	{
		return;
	}
	if (Score < 0)
	{
		SaveGame->BestScoreByDuration.Remove(DurationSeconds);
	}
	else
	{
		SaveGame->BestScoreByDuration.Add(DurationSeconds, Score);
	}
	Save();
}

void URecordsSubsystem::Save()
{
	UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
}
