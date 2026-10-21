// GregrTown - scoring

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GregrSaveGame.generated.h"

/** Persistent player progress. */
UCLASS()
class GREGRTOWN_API UGregrSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** Best score for each challenge duration (seconds). */
	UPROPERTY(SaveGame)
	TMap<int32, int32> BestScoreByDuration;
};
