// GregrTown - UI

#pragma once

#include "CoreMinimal.h"
#include "UI/GregrUserWidget.h"
#include "GregrSettingsWidget.generated.h"

class UButton;
class USlider;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGregrMenuAction);

/** Settings window (also the pause menu): challenge duration, volume, resume / restart. */
UCLASS()
class GREGRTOWN_API UGregrSettingsWidget : public UGregrUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnGregrMenuAction OnResumeRequested;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnGregrMenuAction OnRestartRequested;

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

	void UpdateLabels();

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
};
