// GregrTown - scoring

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RecordsSubsystem.generated.h"

class UGregrSaveGame;

/**
 * Best scores, one per challenge duration (a 10 min record is not a 2 min record).
 * Lives in the game instance so it survives level reloads, saved to disk on every new record.
 */
UCLASS()
class GREGRTOWN_API URecordsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 0 when no record yet for this duration. */
	UFUNCTION(BlueprintPure, Category = "Records")
	int32 GetBestScore(int32 DurationSeconds) const;

	/** Records the score if it beats the best one. Returns true on a new record. */
	UFUNCTION(BlueprintCallable, Category = "Records")
	bool SubmitScore(int32 DurationSeconds, int32 Score);

	/** Tests / debug: force or clear (Score < 0) a record. */
	UFUNCTION(BlueprintCallable, Category = "Records|Debug")
	void SetBestScore(int32 DurationSeconds, int32 Score);

	static const FString SlotName;

protected:
	void Save();

	UPROPERTY(Transient)
	TObjectPtr<UGregrSaveGame> SaveGame;
};
