// GregrTown - game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GregrPlayerController.generated.h"

class AGregrHUD;
class UInputAction;

/**
 * Player controller of GregrTown: game-only input during play, menus (settings / pause,
 * end of challenge) with UI input, and restarting the challenge.
 */
UCLASS(Blueprintable)
class GREGRTOWN_API AGregrPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** Opens the settings window; pauses the game while the challenge runs. */
	UFUNCTION(BlueprintCallable, Category = "Menus")
	void OpenSettings();

	UFUNCTION(BlueprintCallable, Category = "Menus")
	void CloseSettings();

	UFUNCTION(BlueprintCallable, Category = "Menus")
	void ToggleSettings();

	/** Reloads the level for a new challenge. */
	UFUNCTION(BlueprintCallable, Category = "Menus")
	void RestartChallenge();

	UFUNCTION(BlueprintPure, Category = "Menus")
	AGregrHUD* GetGregrHUD() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION()
	void HandleChallengeFinished(int32 FinalScore);

	void SetMenuInputMode(bool bMenu);
	bool IsChallengeFinished() const;

	/** Opens / closes the settings window (works while paused). */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SettingsAction;
};
