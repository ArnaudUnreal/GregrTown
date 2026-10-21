// GregrTown - game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GregrGameMode.generated.h"

/** Base game mode of GregrTown: starts the timed challenge. Pawn class is set in its Blueprint child. */
UCLASS(Blueprintable)
class GREGRTOWN_API AGregrGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGregrGameMode();

	virtual void StartPlay() override;
};
