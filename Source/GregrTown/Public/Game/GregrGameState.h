// GregrTown - game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GregrGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeTimeChanged, int32, SecondsLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeFinished, int32, FinalScore);

UENUM(BlueprintType)
enum class EChallengeEndReason : uint8
{
	TimeUp,
	PlayerDied
};

/** Timed challenge: break as much as possible before the countdown ends. */
UCLASS()
class GREGRTOWN_API AGregrGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Challenge")
	void StartChallenge(float DurationSeconds);

	UFUNCTION(BlueprintPure, Category = "Challenge")
	float GetRemainingTime() const;

	UFUNCTION(BlueprintPure, Category = "Challenge")
	bool IsChallengeRunning() const { return bRunning; }

	UFUNCTION(BlueprintPure, Category = "Challenge")
	bool IsChallengeFinished() const { return bFinished; }

	/** Duration of the current challenge (seconds), the key of its record. */
	UFUNCTION(BlueprintPure, Category = "Challenge")
	int32 GetChallengeDuration() const { return ChallengeDuration; }

	/** Score to beat: best score ever for this challenge duration (0 if none). */
	UFUNCTION(BlueprintPure, Category = "Challenge")
	int32 GetScoreToBeat() const;

	/** Debug / tests: re-read the score to beat from the records. */
	UFUNCTION(BlueprintCallable, Category = "Challenge|Debug")
	void RefreshScoreToBeat();

	/** True once the finished challenge set a new record. */
	UFUNCTION(BlueprintPure, Category = "Challenge")
	bool IsNewRecord() const { return bNewRecord; }

	/** Ends the challenge now (score saved, end screen). The countdown calls it with TimeUp. */
	UFUNCTION(BlueprintCallable, Category = "Challenge")
	void FinishChallenge(EChallengeEndReason Reason);

	UFUNCTION(BlueprintPure, Category = "Challenge")
	EChallengeEndReason GetEndReason() const { return EndReason; }

	/** Debug / tests: jump to the given remaining time. */
	UFUNCTION(BlueprintCallable, Category = "Challenge|Debug")
	void SetRemainingTime(float Seconds);

	/** Fired when the displayed second changes. */
	UPROPERTY(BlueprintAssignable, Category = "Challenge")
	FOnChallengeTimeChanged OnChallengeTimeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Challenge")
	FOnChallengeFinished OnChallengeFinished;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void UpdateChallenge();

	FTimerHandle ChallengeTimer;
	double EndTime = 0.0;
	int32 LastBroadcastSeconds = -1;
	bool bRunning = false;
	bool bFinished = false;
	bool bNewRecord = false;
	EChallengeEndReason EndReason = EChallengeEndReason::TimeUp;
	int32 ChallengeDuration = 0;
	/** Record at the start of the challenge, so beating it during the game does not move the target. */
	int32 ScoreToBeat = 0;
};
