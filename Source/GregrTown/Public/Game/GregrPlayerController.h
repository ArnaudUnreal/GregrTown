// GregrTown - game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GregrPlayerController.generated.h"

class AGregrHUD;
class UInputAction;

/**
 * Player controller of GregrTown: game-only input during play, menus (start, settings / pause,
 * end of challenge) with UI input, restarting the challenge and quitting.
 */
UCLASS(Blueprintable)
class GREGRTOWN_API AGregrPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** Settings window shown before the challenge starts: closing it starts the challenge. */
	UFUNCTION(BlueprintCallable, Category = "Menus")
	void OpenStartMenu();

	/** Opens the settings window; pauses the game while the challenge runs. */
	UFUNCTION(BlueprintCallable, Category = "Menus")
	void OpenSettings();

	/** Closes the settings window: back to the game, to the end screen, or starts the challenge from the start menu. */
	UFUNCTION(BlueprintCallable, Category = "Menus")
	void CloseSettings();

	UFUNCTION(BlueprintCallable, Category = "Menus")
	void ToggleSettings();

	/** Reloads the level for a new challenge, which starts right away. */
	UFUNCTION(BlueprintCallable, Category = "Menus")
	void RestartChallenge();

	/** Leaves the game (ends the session when playing in the editor). */
	UFUNCTION(BlueprintCallable, Category = "Menus")
	void QuitToDesktop();

	UFUNCTION(BlueprintPure, Category = "Menus")
	AGregrHUD* GetGregrHUD() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION()
	void HandleChallengeFinished(int32 FinalScore);

	void SetMenuInputMode(bool bMenu);
	bool IsChallengeFinished() const;
	bool IsChallengeStarted() const;

	/** Opens / closes the settings window (works while paused). */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SettingsAction;
};
