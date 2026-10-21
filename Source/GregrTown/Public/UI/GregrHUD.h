// GregrTown - UI

#pragma once

#include "CoreMinimal.h"
#include "Components/SlateWrapperTypes.h"
#include "GameFramework/HUD.h"
#include "GregrHUD.generated.h"

class UGregrEndScreenWidget;
class UGregrHUDWidget;
class UGregrSettingsWidget;
class USoundBase;

/** Owns the screens of the player: in-game overlay, settings window, end screen. */
UCLASS()
class GREGRTOWN_API AGregrHUD : public AHUD
{
	GENERATED_BODY()

public:
	AGregrHUD();

	/** bStartMenu: before the challenge, the in-game overlay is hidden and the menu offers "Play" instead of "Resume". */
	void ShowSettings(bool bShow, bool bStartMenu = false);
	void ShowEndScreen(int32 FinalScore);

	UFUNCTION(BlueprintPure, Category = "HUD")
	UGregrHUDWidget* GetHUDWidget() const { return HUDWidget; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	UGregrSettingsWidget* GetSettingsWidget() const { return SettingsWidget; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	UGregrEndScreenWidget* GetEndScreenWidget() const { return EndScreenWidget; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	bool IsSettingsVisible() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	bool IsEndScreenVisible() const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleScoreChanged(int32 NewScore, int32 Delta, AActor* Source, AActor* ScoreInstigator, FVector Location);

	UFUNCTION()
	void HandleTimeChanged(int32 SecondsLeft);

	UFUNCTION()
	void HandleChallengeFinished(int32 FinalScore);

	/** Played when points are earned (pitch rises with the amount). */
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Sounds")
	TObjectPtr<USoundBase> ScoreSound;

	/** Played every second of the last CountdownTickSeconds. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Sounds")
	TObjectPtr<USoundBase> CountdownTickSound;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Sounds")
	int32 CountdownTickSeconds = 10;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Sounds")
	TObjectPtr<USoundBase> ChallengeEndSound;

	/** World effect spawned where points are earned (coins...). */
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Effects")
	TObjectPtr<class UNiagaraSystem> ScoreEffect;

	/** Avoids a wall of identical sounds when many objects break at once. */
	double LastScoreSoundTime = -1.0;

	/** Classes can be swapped for Widget Blueprint children to restyle the UI. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UGregrHUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UGregrSettingsWidget> SettingsWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UGregrEndScreenWidget> EndScreenWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UGregrHUDWidget> HUDWidget;

	/** Restored when the start menu closes. */
	ESlateVisibility HUDWidgetVisibility = ESlateVisibility::Visible;

	UPROPERTY(Transient)
	TObjectPtr<UGregrSettingsWidget> SettingsWidget;

	UPROPERTY(Transient)
	TObjectPtr<UGregrEndScreenWidget> EndScreenWidget;
};
