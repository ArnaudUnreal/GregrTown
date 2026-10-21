// GregrTown - scoring

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ScoreSubsystem.generated.h"

/** Location is where the points were earned (floating score text), zero when not relevant. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnScoreChanged, int32, NewScore, int32, Delta, AActor*, Source, AActor*, Instigator, FVector, Location);

/**
 * Holds the score of the current world. Anything can add points (destructibles today,
 * combos/quests/collectibles later) and the HUD only has to listen to OnScoreChanged.
 */
UCLASS()
class GREGRTOWN_API UScoreSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(int32 Points, AActor* Source, AActor* Instigator, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Score")
	void ResetScore();

	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetScore() const { return Score; }

	UPROPERTY(BlueprintAssignable, Category = "Score")
	FOnScoreChanged OnScoreChanged;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	int32 Score = 0;
};
