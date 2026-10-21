// GregrTown - UI

#pragma once

#include "CoreMinimal.h"
#include "UI/GregrSettingsWidget.h"
#include "GregrEndScreenWidget.generated.h"

class UButton;
class UTextBlock;

/** Shown when the countdown ends: final score, replay, settings, quit. */
UCLASS()
class GREGRTOWN_API UGregrEndScreenWidget : public UGregrUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "End Screen")
	void SetFinalScore(int32 Score);

	UFUNCTION(BlueprintPure, Category = "End Screen")
	FText GetScoreDisplay() const;

	UFUNCTION(BlueprintCallable, Category = "End Screen")
	void SetNewRecord(bool bNewRecord);

	UFUNCTION(BlueprintPure, Category = "End Screen")
	bool IsNewRecordShown() const;

	UPROPERTY(BlueprintAssignable, Category = "End Screen")
	FOnGregrMenuAction OnRestartRequested;

	UPROPERTY(BlueprintAssignable, Category = "End Screen")
	FOnGregrMenuAction OnSettingsRequested;

	UPROPERTY(BlueprintAssignable, Category = "End Screen")
	FOnGregrMenuAction OnQuitRequested;

	UFUNCTION(BlueprintPure, Category = "End Screen")
	bool HasQuitButton() const { return QuitButton != nullptr; }

	/** Tests / gamepad helper: same as clicking "Quit". */
	UFUNCTION(BlueprintCallable, Category = "End Screen")
	void ClickQuit() { HandleQuitClicked(); }

protected:
	virtual void BuildDefaultLayout() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleSettingsClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FinalScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NewRecordText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitButton;
};
