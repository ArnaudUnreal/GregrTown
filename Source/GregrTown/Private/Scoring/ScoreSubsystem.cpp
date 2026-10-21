// GregrTown - scoring

#include "Scoring/ScoreSubsystem.h"

void UScoreSubsystem::AddScore(int32 Points, AActor* Source, AActor* Instigator, FVector Location)
{
	if (Points == 0)
	{
		return;
	}

	Score += Points;
	OnScoreChanged.Broadcast(Score, Points, Source, Instigator, Location);
}

void UScoreSubsystem::ResetScore()
{
	const int32 Delta = -Score;
	Score = 0;
	OnScoreChanged.Broadcast(Score, Delta, nullptr, nullptr, FVector::ZeroVector);
}

bool UScoreSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
