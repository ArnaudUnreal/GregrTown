// GregrTown - game

#include "Game/GregrGameState.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Scoring/RecordsSubsystem.h"
#include "Scoring/ScoreSubsystem.h"
#include "TimerManager.h"

void AGregrGameState::StartChallenge(float DurationSeconds)
{
	bRunning = true;
	bFinished = false;
	bNewRecord = false;
	LastBroadcastSeconds = -1;
	ChallengeDuration = FMath::RoundToInt32(FMath::Max(DurationSeconds, 1.f));
	RefreshScoreToBeat();
	// World time stops while the game is paused, so the countdown pauses too.
	EndTime = GetWorld()->GetTimeSeconds() + FMath::Max(DurationSeconds, 1.f);
	GetWorldTimerManager().SetTimer(ChallengeTimer, this, &AGregrGameState::UpdateChallenge, 0.1f, true);
	UpdateChallenge();
}

float AGregrGameState::GetRemainingTime() const
{
	if (!bRunning)
	{
		return 0.f;
	}
	return static_cast<float>(FMath::Max(0.0, EndTime - GetWorld()->GetTimeSeconds()));
}

void AGregrGameState::SetRemainingTime(float Seconds)
{
	if (bRunning)
	{
		EndTime = GetWorld()->GetTimeSeconds() + FMath::Max(Seconds, 0.f);
		UpdateChallenge();
	}
}

void AGregrGameState::UpdateChallenge()
{
	const float Remaining = GetRemainingTime();
	const int32 Seconds = FMath::CeilToInt32(Remaining);
	if (Seconds != LastBroadcastSeconds)
	{
		LastBroadcastSeconds = Seconds;
		OnChallengeTimeChanged.Broadcast(Seconds);
	}

	if (Remaining <= 0.f)
	{
		FinishChallenge();
	}
}

void AGregrGameState::FinishChallenge()
{
	if (bFinished)
	{
		return;
	}

	bRunning = false;
	bFinished = true;
	GetWorldTimerManager().ClearTimer(ChallengeTimer);

	const UScoreSubsystem* Score = UWorld::GetSubsystem<UScoreSubsystem>(GetWorld());
	const int32 FinalScore = Score ? Score->GetScore() : 0;
	if (URecordsSubsystem* Records = GetGameInstance() ? GetGameInstance()->GetSubsystem<URecordsSubsystem>() : nullptr)
	{
		bNewRecord = Records->SubmitScore(ChallengeDuration, FinalScore);
	}
	OnChallengeFinished.Broadcast(FinalScore);
}

int32 AGregrGameState::GetScoreToBeat() const
{
	return ScoreToBeat;
}

void AGregrGameState::RefreshScoreToBeat()
{
	const URecordsSubsystem* Records = GetGameInstance() ? GetGameInstance()->GetSubsystem<URecordsSubsystem>() : nullptr;
	ScoreToBeat = Records ? Records->GetBestScore(ChallengeDuration) : 0;
}

void AGregrGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ChallengeTimer);
	Super::EndPlay(EndPlayReason);
}
