// GregrTown - UI

#pragma once

#include "CoreMinimal.h"
#include "UI/GregrUserWidget.h"
#include "GregrSettingsWidget.generated.h"

class UButton;
class USlider;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGregrMenuAction);

/** Settings window (also the start and pause menu): challenge duration, volume, play / resume, restart, quit. */
UCLASS()
class GREGRTOWN_API UGregrSettingsWidget : public UGregrUserWidget
{
	GENERATED_BODY()

public:
	/** "Play" (start menu) or "Resume" (pause menu). */
	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnGregrMenuAction OnResumeRequested;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnGregrMenuAction OnRestartRequested;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnGregrMenuAction OnQuitRequested;

	/** Start menu: "Play" instead of "Resume", no "Replay", the duration applies to this game. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetStartMenu(bool bStartMenu);

	UFUNCTION(BlueprintPure, Category = "Settings")
	bool IsStartMenu() const { return bIsStartMenu; }

	/** Tests / gamepad helpers: same as clicking the buttons. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ClickResume() { HandleResumeClicked(); }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ClickQuit() { HandleQuitClicked(); }

	/** Label of the play / resume button. */
	UFUNCTION(BlueprintPure, Category = "Settings")
	FText GetResumeLabel() const;

	UFUNCTION(BlueprintPure, Category = "Settings")
	bool HasQuitButton() const { return QuitButton != nullptr; }

	/** Re-reads the saved settings into the controls. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void RefreshFromSettings();

	/** Tests / gamepad helpers: same as moving the sliders. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetDurationFromUI(float Seconds);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetVolumeFromUI(float Volume);

protected:
	virtual void BuildDefaultLayout() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleDurationChanged(float Value);

	UFUNCTION()
	void HandleVolumeChanged(float Value);

	UFUNCTION()
	void HandleResumeClicked();

	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleQuitClicked();

	void UpdateLabels();

	bool bIsStartMenu = false;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> DurationSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DurationLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> VolumeSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> VolumeLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResumeButtonLabel;

	/** "(for the next game)": hidden in the start menu. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DurationHint;
};
