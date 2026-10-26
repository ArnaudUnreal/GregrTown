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
	bUnlimited = DurationSeconds <= 0.f;
	ChallengeDuration = bUnlimited ? 0 : FMath::RoundToInt32(FMath::Max(DurationSeconds, 1.f));
	RefreshScoreToBeat();
	GetWorldTimerManager().ClearTimer(ChallengeTimer);
	if (bUnlimited)
	{
		// No countdown: one broadcast so the HUD shows the challenge has started.
		OnChallengeTimeChanged.Broadcast(0);
		return;
	}
	// World time stops while the game is paused, so the countdown pauses too.
	EndTime = GetWorld()->GetTimeSeconds() + FMath::Max(DurationSeconds, 1.f);
	GetWorldTimerManager().SetTimer(ChallengeTimer, this, &AGregrGameState::UpdateChallenge, 0.1f, true);
	UpdateChallenge();
}

float AGregrGameState::GetRemainingTime() const
{
	if (!bRunning || bUnlimited)
	{
		return 0.f;
	}
	return static_cast<float>(FMath::Max(0.0, EndTime - GetWorld()->GetTimeSeconds()));
}

void AGregrGameState::SetRemainingTime(float Seconds)
{
	if (bRunning && !bUnlimited)
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
		FinishChallenge(EChallengeEndReason::TimeUp);
	}
}

void AGregrGameState::FinishChallenge(EChallengeEndReason Reason)
{
	if (bFinished || !bRunning)
	{
		return;
	}

	bRunning = false;
	bFinished = true;
	EndReason = Reason;
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
